/*
    Jonathan Dummer

    GLFW helper functions

    MIT license
*/

#ifndef HEADER_IMAGE_HELPER
#define HEADER_GLFW_HELPER

#ifdef __cplusplus
extern "C" {
#endif

/**
	This function upscales an image.
	Not to be used to create MIPmaps,
	but to make it square,
	or to make it a power-of-two sized.
**/
int
	up_scale_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int resampled_width, int resampled_height
	);

/**
	This function downscales an image.
	Used for creating MIPmaps,
	the incoming image should be a
	power-of-two sized.
**/
int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	);

/**
	This function takes the RGB components of the image
	and scales each channel from [0,255] to [16,235].
	This makes the colors "Safe" for display on NTSC
	displays.  Note that this is _NOT_ a good idea for
	loading images like normal- or height-maps!
**/
int
	scale_image_RGB_to_NTSC_safe
	(
		unsigned char* orig,
		int width, int height, int channels
	);

#ifdef __cplusplus
}
#endif

#endif /* HEADER_GLFW_HELPER	*/
