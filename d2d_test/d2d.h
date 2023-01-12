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
#include <wrl/client.h>

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
	inline constexpr bool enable_debug_layer = utils::compilation::debug;
	template <typename T>
	using comptr = Microsoft::WRL::ComPtr<T>;
	bool succeeded(HRESULT result) { return SUCCEEDED(result); }
	bool failed   (HRESULT result) { return FAILED   (result); }

	inline std::string hr_to_string(HRESULT hr) noexcept { std::stringstream ss; ss << std::hex << hr; return ss.str(); }

	inline void throw_if_failed(HRESULT hr)
		{
		if (failed(hr))
			{
			throw std::runtime_error{"Error code : " + hr_to_string(hr)};
			}
		}

	template <typename T>
	class wrapper : public comptr<T>
		{
		public:
			using value_type = T;
			using pointer = utils::observer_ptr<value_type>;

			wrapper(const comptr<T>& ptr) : comptr<T>{ptr} { assert(ptr != nullptr); }

			const pointer operator->() const noexcept { return comptr<T>::operator->(); }
			      pointer operator->()       noexcept { return comptr<T>::operator->(); }
			const pointer get       () const noexcept { return comptr<T>::Get       (); }
			      pointer get       ()       noexcept { return comptr<T>::Get       (); }

		protected:
			using wrapper_t = wrapper<T>;
			using comptr_t  = comptr <T>;
		};
	}

namespace co
	{
	struct initializer
		{
		 initializer() { details::throw_if_failed(CoInitialize(nullptr)); } //TODO CoInitializeEx
		~initializer() { CoUninitialize(); }
		};
	}
namespace d3d
	{
	struct create_device_and_context_ret;
	create_device_and_context_ret create_device_and_context();

	class device : public details::wrapper<ID3D11Device2>
		{
		friend create_device_and_context_ret create_device_and_context();
		device(const details::comptr<ID3D11Device>& base) : wrapper_t{create(base)} {}
		inline static comptr_t create(const details::comptr<ID3D11Device>& base)
			{
			comptr_t ret{nullptr};
			details::throw_if_failed(base.As(&ret));
			return ret;
			}
		};
	class context : public details::wrapper<ID3D11DeviceContext3>
		{
		friend create_device_and_context_ret create_device_and_context();
		context(const details::comptr<ID3D11DeviceContext>& base) : wrapper_t{create(base)} {}
		inline static comptr_t create(const details::comptr<ID3D11DeviceContext>& base)
			{
			comptr_t ret{nullptr};
			details::throw_if_failed(base.As(&ret));
			return ret;
			}
		};

	struct create_device_and_context_ret { device device; context context; };
	inline create_device_and_context_ret create_device_and_context()
		{
		details::comptr<ID3D11Device       > device ;
		details::comptr<ID3D11DeviceContext> context;

		UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		if constexpr (details::enable_debug_layer)
			{
			// If the project is in a debug build, enable debugging via SDK Layers with this flag.
			creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
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

		D3D_FEATURE_LEVEL feature_level_created;

		std::array<D3D_DRIVER_TYPE, 3> attempts{D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE};
		HRESULT last{S_FALSE};
		for (const auto& attempt : attempts)
			{
			last = D3D11CreateDevice
				(
				nullptr,                // specify null to use the default adapter
				attempt,
				0,
				creation_flags,         // optionally set debug and Direct2D compatibility flags
				feature_levels.data(),  // list of feature levels this app can support
				feature_levels.size(),  // number of possible feature levels
				D3D11_SDK_VERSION,
				&device,                // returns the Direct3D device created
				&feature_level_created, // returns feature level of device created
				&context                // returns the device immediate context
				);
			if (details::succeeded(last)) { break; }
			}
		details::throw_if_failed(last);

		return {{device}, {context}};
		}
	}

namespace dxgi
	{
	class device : public details::wrapper<IDXGIDevice3>
		{
		public:
			device(const d3d::device& d3d_device) : wrapper_t{create(d3d_device)} {}

		private:
			comptr_t create(const d3d::device& d3d_device)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(d3d_device.As(&ret));
				return ret;
				}
		};

	class swap_chain : public details::wrapper<IDXGISwapChain1>
		{
		public:
			swap_chain(const dxgi::device& dxgi_device, HWND hwnd) : wrapper_t{create(dxgi_device, hwnd)} {}

			void resize(utils::math::vec2u size)
				{
				details::throw_if_failed(get()->ResizeBuffers(2, size.x, size.y, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
				}

			void present()
				{
				details::throw_if_failed(get()->Present(1, 0));
				}

		private:
			inline static comptr_t create(const dxgi::device& dxgi_device, HWND hwnd)
				{
				RECT client_rect{0, 0, 0, 0};
				GetClientRect(hwnd, &client_rect);
				utils::math::rect<long> rectl{.ll{client_rect.left}, .up{client_rect.top}, .rr{client_rect.right}, .dw{client_rect.bottom}};
				
				details::comptr<IDXGIAdapter> dxgi_adapter;
				details::throw_if_failed(dxgi_device->GetAdapter(&dxgi_adapter));

				details::comptr<IDXGIFactory2> dxgi_factory;
				details::throw_if_failed(dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory)));

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

				comptr_t ret{nullptr};
				details::throw_if_failed(dxgi_factory->CreateSwapChainForHwnd(dxgi_device.get(), hwnd, &desc, &desc_fullscreen, nullptr, &ret));

				// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
				// ensures that the application will only render after each VSync, minimizing power consumption.
				dxgi_device->SetMaximumFrameLatency(1);

				return ret;
				}
		};
	}

namespace dw
	{
	class factory : public details::wrapper<IDWriteFactory2>
		{
		public:
			factory() : wrapper_t{create()} {}

		private:
			comptr_t create()
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(DWriteCreateFactory
					(
					DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(value_type),
					&ret
					));
				return ret;
				}
		};

	class text_format : public details::wrapper<IDWriteTextFormat>
		{
		public:
			enum class alignment_ver { top , center, bottom };
			enum class alignment_hor { left, center, justified, right  };
	
			text_format(dw::factory& dw_factory) : wrapper_t{create(dw_factory)} {}
	
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
			inline static pointer create(dw::factory& dw_factory)
				{
				static const WCHAR sc_fontName[] = L"Calibri";
				static const FLOAT sc_fontSize = 50;
	
				pointer ret{nullptr};
				details::throw_if_failed(dw_factory->CreateTextFormat
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
	class imaging_factory : public details::wrapper<IWICImagingFactory2>
		{
		public:
			imaging_factory() : wrapper_t{create()} {}

		private:
			inline static comptr_t create()
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(CoCreateInstance
					(
					CLSID_WICImagingFactory2,
					nullptr,
					CLSCTX_INPROC_SERVER,
					IID_PPV_ARGS(&ret)
					));
				return ret;
				}
		};

	class bitmap : public details::wrapper<IWICBitmap>
		{
		public:
			bitmap(const wic::imaging_factory& wic_imaging_factory, utils::math::vec2u size) : wrapper_t{create(wic_imaging_factory, size)} {}
	
		private:
			inline static comptr_t create(const wic::imaging_factory& wic_imaging_factory, utils::math::vec2u size)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(wic_imaging_factory->CreateBitmap(size.x, size.y, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &ret));
				return ret;
				}
		};
		
	class stream : public details::wrapper<IWICStream>
		{
		public:
			stream(const wic::imaging_factory& wic_imaging_factory, const std::filesystem::path& path) : wrapper_t{create(wic_imaging_factory, path)} {}
	
		private:
			inline static comptr_t create(const wic::imaging_factory& wic_imaging_factory, const std::filesystem::path& path)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(wic_imaging_factory->CreateStream(&ret));
				details::throw_if_failed(ret->InitializeFromFilename(path.wstring().c_str(), GENERIC_WRITE));
				return ret;
				}
		};
	}

namespace d2d
	{
	class factory : public details::wrapper<ID2D1Factory6>
		{
		public:
			factory() : wrapper_t{create()} {}

		private:
			inline static comptr_t create()
				{
				D2D1_FACTORY_OPTIONS options
					{
					.debugLevel{details::enable_debug_layer ? D2D1_DEBUG_LEVEL_INFORMATION : D2D1_DEBUG_LEVEL_NONE}
					};

				comptr_t ret{nullptr};
				details::throw_if_failed(D2D1CreateFactory
					(
					D2D1_FACTORY_TYPE_SINGLE_THREADED,
					__uuidof(value_type),
					&options,
					&ret
					));
				return ret;
				}
		};

	class device : public details::wrapper<ID2D1Device5>
		{
		public:
			device(const d2d::factory& d2d_factory, const dxgi::device& dxgi_device) : wrapper_t{create(d2d_factory, dxgi_device)} {}

		private:
			inline static comptr_t create(const d2d::factory& d2d_factory, const dxgi::device& dxgi_device)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(d2d_factory->CreateDevice(dxgi_device.get(), &ret));
				return ret;
				}
		};

	class device_context : public details::wrapper<ID2D1DeviceContext5>
		{
		public:
			device_context(const d2d::device& d2d_device) : wrapper_t{create(d2d_device)} {}

		private:
			inline static comptr_t create(const d2d::device& d2d_device)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &ret));
				return ret;
				}
		};

	class bitmap : public details::wrapper<ID2D1Bitmap1>
		{
		public:
			bitmap(const d2d::device_context& d2d_device_context, const dxgi::swap_chain& dxgi_swapchain) : wrapper_t{create(d2d_device_context, dxgi_swapchain)} {}
			bitmap(const d2d::device_context& d2d_device_context, utils::math::vec2u size) : wrapper_t{create(d2d_device_context, size)} {}
			//bitmap(const d2d::device_context& d2d_device_context, const wic::bitmap& wic_bitmap) : wrapper_t{create_from_wic_bitmap  (context, wic_bitmap)} {}

		private:
			inline static comptr_t create(const d2d::device_context& d2d_device_context, const dxgi::swap_chain& dxgi_swapchain)
				{
				details::comptr<IDXGISurface2> dxgi_back_buffer;
				details::throw_if_failed(dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer)));

				D2D1_BITMAP_PROPERTIES1 properties
					{
					.pixelFormat{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
					.dpiX{1},//TODO dpi stuff
					.dpiY{1},
					.bitmapOptions{D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW},
					};

				comptr_t ret{nullptr};
				details::throw_if_failed(d2d_device_context->CreateBitmapFromDxgiSurface(dxgi_back_buffer.Get(), &properties, &ret));
				return ret;
				}

			inline static comptr_t create(const d2d::device_context& d2d_device_context)
				{
				D2D1_SIZE_U pixel_size{d2d_device_context->GetPixelSize()};
				return create(d2d_device_context, utils::math::vec2u{pixel_size.width, pixel_size.height});
				}
			
			inline static comptr_t create(const d2d::device_context& d2d_device_context, utils::math::vec2u size)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(d2d_device_context->CreateBitmap
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
			
			//inline static pointer create_from_wic_bitmap(const d2d::device_context& d2d_device_context, const wic::bitmap& wic_bitmap)
			//	{
			//	pointer ret{nullptr};
			//	details::throw_if_failed(d2d_device_context->CreateBitmapFromWicBitmap(wic_bitmap.get(), &ret));
			//	return ret;
			//	}
		};
	}

namespace wic
	{
	class bitmap_encoder : public details::wrapper<IWICBitmapEncoder>
		{
		public:
			bitmap_encoder(const wic::imaging_factory& wic_factory) : wrapper_t{create(wic_factory)} {}

		private:
			inline static comptr_t create(const wic::imaging_factory& wic_factory)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(wic_factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &ret));
				return ret;
				}

		public:
			class frame : public details::wrapper<IWICBitmapFrameEncode>
				{
				public:
					frame(const bitmap_encoder& encoder) : wrapper_t{create(encoder)} {}

				private:
					inline static comptr_t create(const bitmap_encoder& encoder)
						{
						comptr_t ret{nullptr};
						details::throw_if_failed(encoder->CreateNewFrame(&ret, nullptr));
						return ret;
						}
			};
		};
	
	class image_encoder : public details::wrapper<IWICImageEncoder>
		{
		public:
			image_encoder(const wic::imaging_factory& wic_factory, const d2d::device& d2d_device ) : wrapper_t{create(wic_factory, d2d_device )} {}

		private:
			inline static comptr_t create(const wic::imaging_factory& wic_factory, const d2d::device& d2d_device)
				{
				comptr_t ret{nullptr};
				details::throw_if_failed(wic_factory->CreateImageEncoder(d2d_device.get(), &ret));
				return ret;
				}
		};

	void save_to_file(const wic::imaging_factory& wic_factory, const d2d::device& d2d_device, const d2d::bitmap& d2d_bitmap, const std::filesystem::path& path)
		{// https://github.com/uri247/Win81App/blob/master/Direct2D%20save%20to%20image%20file%20sample/C%2B%2B/SaveAsImageFileSample.cpp

		wic::stream stream{wic_factory, path};
		wic::bitmap_encoder bitmap_encoder{wic_factory};
		details::throw_if_failed(bitmap_encoder->Initialize(stream.get(), WICBitmapEncoderNoCache));

		wic::bitmap_encoder::frame frame_encode{bitmap_encoder};
		details::throw_if_failed(frame_encode->Initialize(nullptr));

		wic::image_encoder image_encoder{wic_factory, d2d_device};

		details::throw_if_failed(image_encoder->WriteFrame(d2d_bitmap.get(), frame_encode.get(), nullptr));
		details::throw_if_failed(frame_encode->Commit());
		details::throw_if_failed(bitmap_encoder->Commit());
		details::throw_if_failed(stream->Commit(STGC_DEFAULT));
		}
	}