#pragma once
#include <memory>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#include <utils/oop/disable_move_copy.h>
#include <utils/compilation/debug.h>
#include <utils/math/vec2.h>
#include <utils/math/rect.h>
#include <utils/graphics/colour.h>

#include <d3d11_3.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXMath.h>

#include <utils/memory.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "windowscodecs")

//uses old rendertargets :(
//https://github.com/krish3402/dotnet/blob/d42a2ea76a6d50c6790bbcf88a60de73cce069a9/Samples/Win7Samples/multimedia/Direct2D/SimpleDirect2DApplication/SimpleDirect2dApplication.cpp

namespace details
	{
	inline std::string hr_to_string(HRESULT hr) noexcept { std::stringstream ss; ss << std::hex << hr; return ss.str(); }

	inline void throw_if_failed(HRESULT hr, const std::string& message)
		{
		if (FAILED(hr))
			{
			std::cout << message << "\nError code : " << hr_to_string(hr) << std::endl;
			throw std::runtime_error{message + "\nError code : " + hr_to_string(hr)};
			}
		}
	inline void throw_if_failed(HRESULT hr)
		{
		if (FAILED(hr))
			{
			std::cout << "Error code : " << hr_to_string(hr) << std::endl;
			throw std::runtime_error{"Error code : " + hr_to_string(hr)};
			}
		}
	
	
	template <typename T>
	class wrapper : utils::oop::non_copyable
		{
		public:
			using value_type = T;
			using pointer = utils::observer_ptr<value_type>;
			
			wrapper(pointer ptr) : ptr{ptr} {}

			const pointer operator->() const noexcept { return ptr; }
			      pointer operator->()       noexcept { return ptr; }
			const pointer get       () const noexcept { return ptr; }
			      pointer get       ()       noexcept { return ptr; }

		protected:
			pointer ptr{nullptr};
		};

	template <typename T>
	class wrapper_unique : public wrapper<T>
		{
		using wrapper_t = wrapper<T>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;
			
			wrapper_unique(pointer ptr) : wrapper_t{ptr} {}

			~wrapper_unique() 
				{ 
				if (wrapper<T>::ptr) 
					{
					wrapper<T>::ptr->Release(); 
					} 
				else
					{
					;
					}
				}
		};
	}

namespace d3d11
	{
	struct device_creation_return;
	static device_creation_return create_device();
	
	using device   = details::wrapper_unique<ID3D11Device2       >;
	using context  = details::wrapper_unique<ID3D11DeviceContext3>;

	struct device_creation_return { device device; context context; };
	inline static device_creation_return create_device()
		{
		UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		if constexpr (utils::compilation::debug)
			{
			// If the project is in a debug build, enable debugging via SDK Layers with this flag.
			//creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
			}

		std::array<D3D_FEATURE_LEVEL, 7> feature_levels
			{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
			};

		utils::observer_ptr<ID3D11Device       > device_raw {nullptr};
		utils::observer_ptr<ID3D11DeviceContext> context_raw{nullptr};

		D3D_FEATURE_LEVEL feature_level_created;

		std::array<D3D_DRIVER_TYPE, 3> attempts{D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE};
		HRESULT last;
		for (const auto& attempt : attempts)
			{
			last = D3D11CreateDevice
				(
				nullptr,                  // specify null to use the default adapter
				attempt,
				0,
				creation_flags,           // optionally set debug and Direct2D compatibility flags
				feature_levels.data(),    // list of feature levels this app can support
				feature_levels.size(),    // number of possible feature levels
				D3D11_SDK_VERSION,
				&device_raw,              // returns the Direct3D device created
				&feature_level_created,   // returns feature level of device created
				&context_raw              // returns the device immediate context
				);
			if (SUCCEEDED(last)) { break; }
			}
		details::throw_if_failed(last);

		utils::observer_ptr<ID3D11Device2       > device2_raw {nullptr};
		utils::observer_ptr<ID3D11DeviceContext3> context2_raw{nullptr};

		if (FAILED(device_raw ->QueryInterface(&device2_raw ))) { device_raw->Release(); }
		if (FAILED(context_raw->QueryInterface(&context2_raw))) { device_raw->Release(); context_raw->Release(); }


		return {{device2_raw}, {context2_raw}};
		}
	}

namespace dxgi
	{
	class device : public details::wrapper<IDXGIDevice3>
		{
		using wrapper_t = details::wrapper<IDXGIDevice3>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			device(d3d11::device& d3d_device) : wrapper_t{create(d3d_device)} {}

		private:
			inline static pointer create(d3d11::device& d3d_device)
				{
				pointer ret{nullptr};
				details::throw_if_failed(d3d_device->QueryInterface(&ret));
				ret->SetMaximumFrameLatency(1);//TODO check if necessary
				return ret;
				}
		};

	class adapter : public details::wrapper<IDXGIAdapter>
		{
		using wrapper_t = details::wrapper<IDXGIAdapter>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			adapter(device& device) : wrapper_t{create(device)} {}

		private:
			inline static pointer create(device& device)
				{
				pointer ret{nullptr};
				details::throw_if_failed(device->GetAdapter(&ret));
				return ret;
				}
		};

	class factory : public details::wrapper<IDXGIFactory3>
		{
		using wrapper_t = details::wrapper<IDXGIFactory3>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			factory(adapter& adapter) : wrapper_t{create(adapter)} {}

		private:
			inline static pointer create(adapter& adapter)
				{
				pointer ret{nullptr};
				details::throw_if_failed(adapter->GetParent(IID_PPV_ARGS(&ret)));
				return ret;
				}
		};
	
	class swap_chain : public details::wrapper_unique<IDXGISwapChain1>
		{
		using wrapper_t = details::wrapper_unique<IDXGISwapChain1>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			swap_chain(device& dxgi_device, HWND hwnd) : wrapper_t{create(dxgi_device, hwnd)} {}

			void invalidate() noexcept { ptr = nullptr; }
			void recreate(utils::math::vec2u size, device& dxgi_device, HWND hwnd)
				{
				if (ptr)
					{
					ptr->ResizeBuffers(2, size.x, size.y, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
					}
				else
					{
					ptr = create(dxgi_device, hwnd);
					}
				}

		private:
			inline static pointer create(device& dxgi_device, HWND hwnd)
				{
				RECT client_rect{0, 0, 0, 0};
				details::throw_if_failed(GetClientRect(hwnd, &client_rect));
				utils::math::rect<long> rectl{.ll{client_rect.left}, .up{client_rect.top}, .rr{client_rect.right}, .dw{client_rect.bottom}};

				dxgi::adapter dxgi_adapter{dxgi_device};
				dxgi::factory dxgi_factory{dxgi_adapter};//Raymond's is IDXGIFactory2 not IDXGIFactory3 here

				DXGI_SWAP_CHAIN_DESC1 desc
					{
					.Width      {static_cast<UINT>(rectl.w())},
					.Height     {static_cast<UINT>(rectl.h())},
					.Format     {DXGI_FORMAT_B8G8R8A8_UNORM},
					.Stereo     {false},
					.SampleDesc
						{
						.Count  {1},
						.Quality{0}
						},
					.BufferUsage {DXGI_USAGE_RENDER_TARGET_OUTPUT},
					.BufferCount {2},
					.Scaling     {DXGI_SCALING_NONE},
					.SwapEffect  {DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL},
					.AlphaMode   {DXGI_ALPHA_MODE_IGNORE},
					.Flags       {0},
					};
				DXGI_SWAP_CHAIN_FULLSCREEN_DESC desc_fullscreen
					{
					.RefreshRate{.Numerator{1}, .Denominator{0}},
					.Scaling     {DXGI_MODE_SCALING_CENTERED},
					};

				pointer ret{nullptr};
				details::throw_if_failed(dxgi_factory->CreateSwapChainForHwnd(dxgi_device.get(), hwnd, &desc, &desc_fullscreen, nullptr, &ret));

				dxgi_device->SetMaximumFrameLatency(1);

				return ret;
				}
		};
	}

namespace dw
	{
	class factory : public details::wrapper_unique<IDWriteFactory2>
		{
		using wrapper_t = details::wrapper_unique<IDWriteFactory2>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			factory(D2D1_FACTORY_TYPE factory_type = D2D1_FACTORY_TYPE_SINGLE_THREADED) : wrapper_t{create(factory_type)} {}


		private:
			inline static pointer create(D2D1_FACTORY_TYPE factory_type)
				{
				pointer ret{nullptr};
				details::throw_if_failed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(ret), reinterpret_cast<IUnknown**>(&ret)));
				return ret;
				}
		};
	class text_format : public details::wrapper_unique<IDWriteTextFormat>
		{
		using wrapper_t = details::wrapper_unique<IDWriteTextFormat>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			enum class alignment_ver { top , center, bottom };
			enum class alignment_hor { left, center, justified, right  };

			text_format(factory& factory) : wrapper_t{create(factory)} {}

			void set_alignment_hor(alignment_hor alignment)
				{
				switch (alignment)
					{
					case dw::text_format::alignment_hor::left     : get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING  ); break;
					case dw::text_format::alignment_hor::center   : get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER   ); break;
					case dw::text_format::alignment_hor::justified: get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_JUSTIFIED); break;
					case dw::text_format::alignment_hor::right    : get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING ); break;
					}
				}
			void set_alignment_ver(alignment_ver alignment)
				{
				switch (alignment)
					{
					case dw::text_format::alignment_ver::top      : get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_NEAR  ); break;
					case dw::text_format::alignment_ver::center   : get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER); break;
					case dw::text_format::alignment_ver::bottom   : get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_FAR   ); break;
					}
				}

		private:
			inline static pointer create(factory& factory)
				{
				static const WCHAR sc_fontName[] = L"Calibri";
				static const FLOAT sc_fontSize = 50;

				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateTextFormat
					(
					sc_fontName,
					nullptr,
					DWRITE_FONT_WEIGHT_NORMAL,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					sc_fontSize,
					L"", //locale
					&ret
					));
				return ret;
				}
		};
	}

namespace wic
	{
	class factory : public details::wrapper_unique<IWICImagingFactory2>
		{
		using wrapper_t = details::wrapper_unique<IWICImagingFactory2>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			factory() : wrapper_t{create()} {}

		private:
			inline static pointer create()
				{
				pointer ret{nullptr};

				details::throw_if_failed(CoInitialize(NULL));
				details::throw_if_failed(CoCreateInstance
					(
					CLSID_WICImagingFactory2,
					nullptr,
					CLSCTX_INPROC_SERVER,
					IID_IWICImagingFactory,
					reinterpret_cast<void**>(&ret)
					));
				return ret;
				}
		};

	class bitmap : public details::wrapper_unique<IWICBitmap>
		{
		using wrapper_t = details::wrapper_unique<IWICBitmap>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			bitmap(factory& factory, utils::math::vec2u size) : wrapper_t{create(factory, size)} {}

		private:
			inline static pointer create(factory& factory, utils::math::vec2u size)
				{
				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateBitmap(size.x, size.y, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &ret));
				return ret;
				}
		};
	
	class stream : public details::wrapper_unique<IWICStream>
		{
		using wrapper_t = details::wrapper_unique<IWICStream>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			stream(factory& factory, const std::filesystem::path& path) : wrapper_t{create(factory, path)} {}

		private:
			inline static pointer create(factory& factory, const std::filesystem::path& path)
				{
				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateStream(&ret));
				details::throw_if_failed(ret    ->InitializeFromFilename(path.wstring().c_str(), GENERIC_WRITE));
				return ret;
				}
		};
	}

namespace d2d
	{
	class factory : public details::wrapper_unique<ID2D1Factory6>
		{
		using wrapper_t = details::wrapper_unique<ID2D1Factory6>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			factory(D2D1_FACTORY_TYPE factory_type = D2D1_FACTORY_TYPE_SINGLE_THREADED) : wrapper_t{create(factory_type)} {}

		private:
			inline static pointer create(D2D1_FACTORY_TYPE factory_type)
				{
				D2D1_FACTORY_OPTIONS options{};
				if constexpr (utils::compilation::debug)
					{
					// If the project is in a debug build, enable debugging via SDK Layers with this flag.
					options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
					}

				pointer ret{nullptr};
				details::throw_if_failed(D2D1CreateFactory(factory_type, __uuidof(ID2D1Factory6), &options, reinterpret_cast<void**>(&ret)));
				return ret;
				}
		};

	class render_target : public details::wrapper_unique<ID2D1RenderTarget>
		{
		using wrapper_t = details::wrapper_unique<ID2D1RenderTarget>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			render_target from_bitmap(factory& factory, wic::bitmap& bitmap) { return {factory, bitmap}; }

			render_target(factory& factory, wic::bitmap& bitmap) : wrapper_t{create_from_bitmap(factory, bitmap)} {}

		private:
			inline static pointer create_from_bitmap(factory& factory, wic::bitmap& bitmap)
				{
				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateWicBitmapRenderTarget(bitmap.get(), D2D1::RenderTargetProperties(), &ret));
				return ret;
				}
		};

	class device : public details::wrapper_unique<ID2D1Device5>
		{//https://learn.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
		using wrapper_t = details::wrapper_unique<ID2D1Device5>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			device(factory& factory, dxgi::device& d3d_device) : wrapper_t{create(factory, d3d_device)} {}

		private:
			inline static pointer create(factory& factory, dxgi::device& dxgi_device)
				{
				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateDevice(dxgi_device.get(), &ret));
				return ret;
				}
		};
	class context : public details::wrapper_unique<ID2D1DeviceContext5>
		{//https://learn.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
		using wrapper_t = details::wrapper_unique<ID2D1DeviceContext5>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			context(device& device) : wrapper_t{create(device)} {}

		private:
			inline static pointer create(device& device)
				{
				pointer ret{nullptr};
				details::throw_if_failed(device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE , &ret));
				return ret;
				}
		};

	class bitmap : public details::wrapper_unique<ID2D1Bitmap1>
		{//https://learn.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
		using wrapper_t = details::wrapper_unique<ID2D1Bitmap1>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			bitmap(context& context, utils::math::vec2u size) : wrapper_t{create(context, size)} {}
			bitmap(context& context) : wrapper_t{create(context)} {}
			bitmap(context& context, wic::bitmap     & wic_bitmap) : wrapper_t{create_from_wic_bitmap  (context, wic_bitmap)} {}
			bitmap(context& context, dxgi::swap_chain& swap_chain) : wrapper_t{create_from_dxgi_surface(context, swap_chain)} {}

			inline static bitmap from_wic_bitmap  (context& context, wic::bitmap     & wic_bitmap) { return {context, wic_bitmap}; }
			inline static bitmap from_dxgi_surface(context& context, dxgi::swap_chain& swap_chain) { return {context, swap_chain}; }

		private:
			inline static pointer create(context& context)
				{
				D2D1_SIZE_U pixel_size{context->GetPixelSize()};
				return create(context, utils::math::vec2u{pixel_size.width, pixel_size.height});
				}

			inline static pointer create(context& context, utils::math::vec2u size)
				{
				pointer ret{nullptr};
				details::throw_if_failed(context->CreateBitmap
					(
					D2D1_SIZE_U{.width{size.x}, .height{size.y}},
					nullptr, 
					size.x * 4,
					D2D1_BITMAP_PROPERTIES1
						{
						.pixelFormat{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
						.bitmapOptions{D2D1_BITMAP_OPTIONS_TARGET},
						},
					&ret
					));
				return ret;
				}

			inline static pointer create_from_wic_bitmap(context& context, wic::bitmap& wic_bitmap)
				{
				pointer ret{nullptr};
				details::throw_if_failed(context->CreateBitmapFromWicBitmap(wic_bitmap.get(), &ret));
				return ret;
				}

			inline static pointer create_from_dxgi_surface(context& context, dxgi::swap_chain& swap_chain)
				{
				utils::observer_ptr<IDXGISurface2> dxgi_back_buffer;
				details::throw_if_failed(swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer)));

				D2D1_BITMAP_PROPERTIES1 properties
					{
					.pixelFormat{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
					.dpiX{1},//TODO dpi stuff
					.dpiY{1},
					.bitmapOptions{D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW},
					};

				pointer ret{nullptr};
				details::throw_if_failed(context->CreateBitmapFromDxgiSurface(dxgi_back_buffer, &properties, &ret));
				return ret;
				}
		};
	}

namespace wic
	{
	class bitmap_encoder : public details::wrapper_unique<IWICBitmapEncoder>
		{
		using wrapper_t = details::wrapper_unique<IWICBitmapEncoder>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			bitmap_encoder(factory& factory) : wrapper_t{create(factory)} {}

		private:
			inline static pointer create(factory& factory)
				{
				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &ret));
				return ret;
				}

		public:
			class frame : public details::wrapper_unique<IWICBitmapFrameEncode>
				{
				using wrapper_t = details::wrapper_unique<IWICBitmapFrameEncode>;
				public:
					using value_type = wrapper_t::value_type;
					using pointer    = wrapper_t::pointer;

					frame(bitmap_encoder& encoder) : wrapper_t{create(encoder)} {}

				private:
					inline static pointer create(bitmap_encoder& encoder)
						{
						pointer ret{nullptr};
						details::throw_if_failed(encoder->CreateNewFrame(&ret, nullptr));
						return ret;
						}
			};
		};
	
	class image_encoder : public details::wrapper_unique<IWICImageEncoder>
		{
		using wrapper_t = details::wrapper_unique<IWICImageEncoder>;
		public:
			using value_type = wrapper_t::value_type;
			using pointer    = wrapper_t::pointer;

			image_encoder(factory& factory, d2d::device & d2d_device ) : wrapper_t{create(factory, d2d_device )} {}

		private:
			inline static pointer create(factory& factory, d2d::device& d2d_device)
				{
				pointer ret{nullptr};
				details::throw_if_failed(factory->CreateImageEncoder(d2d_device.get(), &ret));
				return ret;
				}
		};

	void save_to_file(factory& factory, d2d::device& d2d_device, d2d::context& d2d_context, d2d::bitmap& d2d_bitmap, const std::filesystem::path& path)
		{// https://github.com/uri247/Win81App/blob/master/Direct2D%20save%20to%20image%20file%20sample/C%2B%2B/SaveAsImageFileSample.cpp

		stream stream{factory, path};
		bitmap_encoder bitmap_encoder{factory};
		details::throw_if_failed(bitmap_encoder->Initialize(stream.get(), WICBitmapEncoderNoCache));

		bitmap_encoder::frame frame_encode{bitmap_encoder};
		details::throw_if_failed(frame_encode->Initialize(nullptr));

		image_encoder image_encoder{factory, d2d_device};

		details::throw_if_failed(image_encoder->WriteFrame(d2d_bitmap.get(), frame_encode.get(), nullptr));
		details::throw_if_failed(frame_encode->Commit());
		details::throw_if_failed(bitmap_encoder->Commit());
		details::throw_if_failed(stream->Commit(STGC_DEFAULT));
		}
	}