#pragma once

#include <utils/win32/window/window.h>
#include <utils/win32/window/style.h>
#include "d2d.h"

namespace d2d
	{
	class frame;

	class window : public utils::win32::window::module
		{
		friend class frame;

		public:
			using on_draw_signature = void(window&, const d2d::device_context&);

			struct create_info
				{
				const d3d ::device& d3d_device;
				const d2d ::device& d2d_device;
				const dxgi::device& dxgi_device;
				std ::function<on_draw_signature> on_render;
				};

			window(utils::win32::window::base& base, create_info create_info) :
				module
					{
					base,
					[this](UINT msg, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> { return procedure(msg, wparam, lparam); }
					},
				on_render{create_info.on_render},
				d2d_device_context{create_info.d2d_device},
				dxgi_swapchain{create_info.dxgi_device, get_base().get_handle()},
				d2d_bitmap_target{d2d_device_context, dxgi_swapchain}
				{
				d2d_device_context->SetTarget(d2d_bitmap_target.get());
				}

			std::function<on_draw_signature> on_render;

			void present() noexcept 
				{
				dxgi_swapchain.present();
				}

		private:
			d2d::device_context d2d_device_context;
			dxgi::swap_chain dxgi_swapchain;
			d2d::bitmap d2d_bitmap_target;


			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
				{
				switch (msg)
					{
					case WM_SIZE:
						on_resize({LOWORD(lparam), HIWORD(lparam)});
						break;

					case WM_DISPLAYCHANGE:
						//InvalidateRect(get_handle(), NULL, FALSE);
						break;

					case WM_PAINT:
						if (on_render)
							{
							on_render(*this, d2d_device_context);
							ValidateRect(get_base().get_handle(), NULL);
							return 0;
							}
						break;

					}

				return std::nullopt;
				}

			void on_resize(utils::math::vec2u size)
				{
				if (dxgi_swapchain.get() == nullptr)
					{
					d2d_device_context->SetTarget(nullptr);
					dxgi_swapchain.resize(size);
					d2d_bitmap_target = d2d::bitmap{d2d_device_context, dxgi_swapchain};
					d2d_device_context->SetTarget(d2d_bitmap_target.get());
					dxgi_swapchain.present();
					}
				}
		};
	}