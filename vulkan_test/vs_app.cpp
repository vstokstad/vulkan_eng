#include "vs_app.h"

namespace vs
{
	vs_app::vs_app( int window_width_pixels ) : width(window_width_pixels){
	}

	vs_app::vs_app( float aspect_ratio, int window_width_pixels ) : aspect_ratio(aspect_ratio),
	                                                                width(window_width_pixels),
	                                                                height(width / this->aspect_ratio){
	}

	vs_app::vs_app( int window_width_pixels, int window_height_pixels ) : aspect_ratio(1.0),
	                                                                      width(window_width_pixels),
	                                                                      height(window_height_pixels){
	}

	void vs_app::run() const{
		while ( vs_window.should_close() ){
			glfwPollEvents();
		}
	}
}
