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
			struct create_info
				{
				d3d11::device & d3d_device;
				dxgi ::device & dxgi_device;
				d2d  ::context& d2d_context;
				std::function<void(window&)> on_render;
				};

			window(utils::win32::window::base& base, create_info create_info) :
				module
					{
					base,
					[this](UINT msg, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> { return procedure(msg, wparam, lparam); }
					},
				dxgi_swapchain{create_info.dxgi_device, get_base().get_handle()},
				target{create_info.d2d_context, dxgi_swapchain},
				on_render     {create_info.on_render},
				dxgi_device_ptr{&create_info.dxgi_device}
				{
				}

			std::function<void(window&)> on_render;

			const d2d::bitmap& get_render_target() const noexcept { return target; }
			      d2d::bitmap& get_render_target()       noexcept { return target; }

			void present() noexcept 
				{
				dxgi_swapchain->Present(1, 0);
				}

		private:
			dxgi::swap_chain dxgi_swapchain;
			d2d::bitmap target;
			utils::observer_ptr<dxgi::device> dxgi_device_ptr;


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
							on_render(*this);
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
					dxgi_swapchain.recreate(size, *dxgi_device_ptr, get_base().get_handle());
					}
				}
		};
	}