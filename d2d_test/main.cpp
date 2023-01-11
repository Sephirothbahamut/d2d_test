#include "d2d.h"
#include "window.h"

ID2D1BitmapBrush* CreateGridPatternBrush(d2d::context& d2d_context)
	{
	ID2D1BitmapBrush* ret{nullptr};

	// Create a compatible render target.

	d2d::bitmap d2d_bitmap{d2d_context, {10, 10}};

	ID2D1Image* previous_target;
	d2d_context->GetTarget(&previous_target);
	d2d_context->SetTarget(d2d_bitmap.get());

	// Draw a pattern.
	ID2D1SolidColorBrush* pGridBrush{nullptr};
	details::throw_if_failed(d2d_context->CreateSolidColorBrush
		(
		D2D1::ColorF(D2D1::ColorF(0.93f, 0.94f, 0.96f, 1.0f)),
		&pGridBrush
		));

	d2d_context->BeginDraw();
	d2d_context->FillRectangle(D2D1::RectF(0.0f, 0.0f, 10.0f, 1.0f), pGridBrush);
	d2d_context->FillRectangle(D2D1::RectF(0.0f, 0.1f, 1.0f, 10.0f), pGridBrush);
	d2d_context->EndDraw();

	// Choose the tiling mode for the bitmap brush.
	D2D1_BITMAP_BRUSH_PROPERTIES brushProperties{D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP)};

	// Create the bitmap brush.
	details::throw_if_failed(d2d_context->CreateBitmapBrush(d2d_bitmap.get(), brushProperties, &ret));

	pGridBrush->Release();

	d2d_context->SetTarget(previous_target);

	return ret;
	}

struct qwindow : utils::win32::window::base
	{
	struct create_info
		{
		utils::win32::window::base::create_info  base{};
		utils::win32::window::style::create_info style{};
		d2d::window::create_info                 d2d;
		};

	qwindow(create_info& create_info) : 
		utils::win32::window::base{create_info.style.adjust_base_create_info(create_info.base)},
		style{*this, create_info.style},
		d2d  {*this, create_info.d2d  }
		{}
	
	utils::win32::window::style style;
	d2d::window d2d;
	};


void inner()
	{
	// CreateDeviceIndependentResources
	d2d::factory d2d_factory;
	dw ::factory dw_factory;
	wic::factory wic_factory;

	// CreateDeviceResources
	auto [d3d_device, d3d_context] {d3d11::create_device()};
	dxgi::device dxgi_device{d3d_device};
	d2d::device  d2d_device {d2d_factory, dxgi_device};
	d2d::context d2d_context{d2d_device};

	// Grayscale text anti-aliasing is recommended for all Windows Runtime apps.
	d2d_context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);

	dw::text_format dw_text_format{dw_factory};

	auto* m_pGridPatternBitmapBrush{CreateGridPatternBrush(d2d_context)};

	//auto on_render{[&](d2d::window& w)
	//	{
	//	d2d_context->SetTarget(w.get_render_target().get());
	//	
	//	d2d_context->BeginDraw();
	//	d2d_context->SetTransform(D2D1::Matrix3x2F::Identity());
	//	d2d_context->Clear(D2D1_COLOR_F{D2D1::ColorF::Red});//D2D1_COLOR_F{.r{1.f}, .g{0.f}, .b{0.f}, .a{.5f}});
	//
	//	d2d_context->FillRectangle(D2D1::RectF(0.0f, 0.0f, w.get_base().client_rect.width(), w.get_base().client_rect.height()), m_pGridPatternBitmapBrush);
	//
	//	d2d_context->EndDraw();
	//	
	//	w.present();
	//	}};
	//
	//utils::win32::window::initializer win_init;
	//qwindow::create_info qreate_info
	//	{
	//	.d2d
	//		{
	//		.d3d_device  {d3d_device},
	//		.dxgi_device {dxgi_device},
	//		.d2d_context {d2d_context},
	//		.on_render   {on_render}
	//		}
	//	};
	//qwindow window{qreate_info};
	//
	//window.show();
	//while (window.is_open())
	//	{
	//	while (window.poll_event());
	//	RedrawWindow(window.get_handle(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	//	}

	d2d::bitmap d2d_bitmap{d2d_context, utils::math::vec2u{300, 650}}; //Draw on this to save to image


	d2d_context->SetTarget(d2d_bitmap.get());

	d2d_context->BeginDraw();
	d2d_context->SetTransform(D2D1::Matrix3x2F::Identity());
	d2d_context->Clear(D2D1_COLOR_F{D2D1::ColorF::Red});

	d2d_context->FillRectangle(D2D1::RectF(0.0f, 0.0f, 12, 32), m_pGridPatternBitmapBrush);

	d2d_context->EndDraw();
	wic::save_to_file(wic_factory, d2d_device, d2d_context, d2d_bitmap, "out.png");

	m_pGridPatternBitmapBrush->Release();
	}

int main()
	{
	///*
	try { inner(); }
	/*/
	inner(); try {}
	/**/
	catch (const std::exception& e)
		{
		std::cout << e.what() << std::endl;
		}
	}