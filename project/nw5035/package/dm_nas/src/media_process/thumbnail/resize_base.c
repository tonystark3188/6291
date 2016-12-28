#include "resize_base.h"

/* 
 *Desc: find valud most valied bit!
 *
 *Return: return the bit position;
 */
static uint8_t _find_bit(uint16_t val)
{
	uint8_t i = ((sizeof(uint8_t) << 3) - 1);

	while(i > 0)
	{
		if(BIT(val, i))
			break;
		--i;
	}

	return i;
}

/*
 *Desc: according to input and the max output images, cal the right scale nums. 
 *
 *src_width: input, input image width
 *src_height: input, input image height
 *p_request: input, the max output image rqeuset. 
 *
 *Return: the scale number will be the power of 2!
 */
uint16_t _find_scale_factor(uint32_t src_width, uint32_t src_height, 
                                    ImgReduceRequest *p_request)
{
	uint16_t scale = 1; 

	if(p_request->scale == 0)
	{
		uint16_t tmp1 = 1;
		uint16_t tmp2 = 1;
		tmp1 = src_width / p_request->width;		
		tmp2 = src_height / p_request->height;

		tmp1 = MIN(tmp1, tmp2);
		scale = _find_bit(tmp1);
	}
	else
	{
		scale = _find_bit(p_request->scale);	
	}

	scale = (0x1 << scale);
	return scale;
}



