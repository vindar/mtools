#
# look for libjpeg
#

find_package(JPEG REQUIRED)


if (JPEG_FOUND)
	set (JPEG_alt_FOUND 1)
endif()