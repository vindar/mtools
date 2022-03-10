/** @file palette.cpp */
//
// Copyright 2015 Arvind Singh
//
// This file is part of the mtools library.
//
// mtools is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mtools  If not, see <http://www.gnu.org/licenses/>.


#include "graphics/palette.hpp"

namespace mtools
{


	namespace Palette
	{

		// unicolor palette

		const ColorPalette Blue = { 9,  {RGBc(247,251,255),RGBc(222,235,247),RGBc(198,219,239),RGBc(158,202,225),RGBc(107,174,214),RGBc(66,146,198),RGBc(33,113,181),RGBc(8,81,156),RGBc(8,48,107) } };
		const ColorPalette Green = { 9, {RGBc(247,252,245),RGBc(229,245,224),RGBc(199,233,192),RGBc(161,217,155),RGBc(116,196,118),RGBc(65,171,93),RGBc(35,139,69),RGBc(0,109,44),RGBc(0,68,27) } };
		const ColorPalette Black = { 9, {RGBc(255,255,255),RGBc(240,240,240),RGBc(217,217,217),RGBc(189,189,189),RGBc(150,150,150),RGBc(115,115,115),RGBc(82,82,82),RGBc(37,37,37),RGBc(0,0,0) } };
		const ColorPalette Orange = { 9, {RGBc(255,245,235),RGBc(254,230,206),RGBc(253,208,162),RGBc(253,174,107),RGBc(253,141,60),RGBc(241,105,19),RGBc(217,72,1),RGBc(166,54,3),RGBc(127,39,4) } };
		const ColorPalette Violet = { 9, {RGBc(252,251,253),RGBc(239,237,245),RGBc(218,218,235),RGBc(188,189,220),RGBc(158,154,200),RGBc(128,125,186),RGBc(106,81,163),RGBc(84,39,143),RGBc(63,0,125) } };
		const ColorPalette Red = { 9, {RGBc(255,245,240),RGBc(254,224,210),RGBc(252,187,161),RGBc(252,146,114),RGBc(251,106,74),RGBc(239,59,44),RGBc(203,24,29),RGBc(165,15,21),RGBc(103,0,13) } };


		// unicolor to white / Yellow

		const ColorPalette Yellow_to_Red = { 9, {RGBc(255,255,204),RGBc(255,237,160),RGBc(254,217,118),RGBc(254,178,76),RGBc(253,141,60),RGBc(252,78,42),RGBc(227,26,28),RGBc(189,0,38),RGBc(128,0,38) } };
		const ColorPalette Yellow_to_Blue = { 9, {RGBc(255,255,217),RGBc(237,248,177),RGBc(199,233,180),RGBc(127,205,187),RGBc(65,182,196),RGBc(29,145,192),RGBc(34,94,168),RGBc(37,52,148),RGBc(8,29,88) } };
		const ColorPalette Yellow_to_Green = { 9, {RGBc(255,255,229),RGBc(247,252,185),RGBc(217,240,163),RGBc(173,221,142),RGBc(120,198,121),RGBc(65,171,93),RGBc(35,132,67),RGBc(0,104,55),RGBc(0,69,41) } };
		const ColorPalette White_to_Green = { 9, {RGBc(247,252,253),RGBc(229,245,249),RGBc(204,236,230),RGBc(153,216,201),RGBc(102,194,164),RGBc(65,174,118),RGBc(35,139,69),RGBc(0,109,44),RGBc(0,68,27) } };
		const ColorPalette White_to_Violet = { 9, {RGBc(247,252,253),RGBc(224,236,244),RGBc(191,211,230),RGBc(158,188,218),RGBc(140,150,198),RGBc(140,107,177),RGBc(136,65,157),RGBc(129,15,124),RGBc(77,0,75) } };
		const ColorPalette White_to_Blue = { 9, {RGBc(255,247,251),RGBc(236,231,242),RGBc(208,209,230),RGBc(166,189,219),RGBc(116,169,207),RGBc(54,144,192),RGBc(5,112,176),RGBc(4,90,141),RGBc(2,56,88) } };


		// diverging palette (between colors)

		const ColorPalette Red_to_Violet = { 11, {RGBc(158,1,66),RGBc(213,62,79),RGBc(244,109,67),RGBc(253,174,97),RGBc(254,224,139),RGBc(255,255,191),RGBc(230,245,152),RGBc(171,221,164),RGBc(102,194,165),RGBc(50,136,189),RGBc(94,79,162) } };
		const ColorPalette Red_to_Green = { 11, {RGBc(165,0,38),RGBc(215,48,39),RGBc(244,109,67),RGBc(253,174,97),RGBc(254,224,139),RGBc(255,255,191),RGBc(217,239,139),RGBc(166,217,106),RGBc(102,189,99),RGBc(26,152,80),RGBc(0,104,55) } };
		const ColorPalette Red_to_Blue = { 11, {RGBc(165,0,38),RGBc(215,48,39),RGBc(244,109,67),RGBc(253,174,97),RGBc(254,224,144),RGBc(255,255,191),RGBc(224,243,248),RGBc(171,217,233),RGBc(116,173,209),RGBc(69,117,180),RGBc(49,54,149) } };
		const ColorPalette Red_to_Black = { 11, {RGBc(103,0,31),RGBc(178,24,43),RGBc(214,96,77),RGBc(244,165,130),RGBc(253,219,199),RGBc(255,255,255),RGBc(224,224,224),RGBc(186,186,186),RGBc(135,135,135),RGBc(77,77,77),RGBc(26,26,26) } };
		const ColorPalette Maroon_to_Violet = { 11, {RGBc(127,59,8),RGBc(179,88,6),RGBc(224,130,20),RGBc(253,184,99),RGBc(254,224,182),RGBc(247,247,247),RGBc(216,218,235),RGBc(178,171,210),RGBc(128,115,172),RGBc(84,39,136),RGBc(45,0,75) } };
		const ColorPalette Violet_to_Green = { 11, {RGBc(142,1,82),RGBc(197,27,125),RGBc(222,119,174),RGBc(241,182,218),RGBc(253,224,239),RGBc(247,247,247),RGBc(230,245,208),RGBc(184,225,134),RGBc(127,188,65),RGBc(77,146,33),RGBc(39,100,25) } };
		const ColorPalette Maroon_to_blue = { 11, {RGBc(84,48,5),RGBc(140,81,10),RGBc(191,129,45),RGBc(223,194,125),RGBc(246,232,195),RGBc(245,245,245),RGBc(199,234,229),RGBc(128,205,193),RGBc(53,151,143),RGBc(1,102,94),RGBc(0,60,48) } };

		const ColorPalette matlabJet = { 72, {RGBc(0, 0, 127), RGBc(0, 0, 141), RGBc(0, 0, 155), RGBc(0, 0, 169), RGBc(0, 0, 183), RGBc(0, 0, 198), RGBc(0, 0, 212), RGBc(0, 0, 226), RGBc(0, 0, 240),
											 RGBc(0, 0, 255), RGBc(0, 14, 255), RGBc(0, 28, 255), RGBc(0, 42, 255), RGBc(0, 56, 255), RGBc(0, 70, 255), RGBc(0, 84, 255), RGBc(0, 98, 255), RGBc(0, 112, 255),
											 RGBc(0, 127, 255), RGBc(0, 141, 255), RGBc(0, 155, 255), RGBc(0, 169, 255), RGBc(0, 183, 255), RGBc(0, 198, 255), RGBc(0, 212, 255), RGBc(0, 226, 255), RGBc(0, 240, 255),
											 RGBc(0, 255, 255), RGBc(14, 255, 240), RGBc(28, 255, 226), RGBc(42, 255, 212), RGBc(56, 255, 198), RGBc(70, 255, 183), RGBc(84, 255, 169), RGBc(98, 255, 155), RGBc(112, 255, 141),
											 RGBc(127, 255, 127), RGBc(141, 255, 112), RGBc(155, 255, 98), RGBc(169, 255, 84), RGBc(183, 255, 70), RGBc(198, 255, 56), RGBc(212, 255, 42), RGBc(226, 255, 28), RGBc(240, 255, 14),
											 RGBc(255, 255, 0), RGBc(255, 240, 0), RGBc(255, 226, 0), RGBc(255, 212, 0), RGBc(255, 198, 0), RGBc(255, 183, 0), RGBc(255, 169, 0), RGBc(255, 155, 0), RGBc(255, 141, 0),
											 RGBc(255, 127, 0), RGBc(255, 112, 0), RGBc(255, 98, 0), RGBc(255, 84, 0), RGBc(255, 70, 0), RGBc(255, 56, 0), RGBc(255, 42, 0), RGBc(255, 28, 0), RGBc(255, 14, 0),
											 RGBc(255, 0, 0), RGBc(240, 0, 0), RGBc(226, 0, 0), RGBc(212, 0, 0), RGBc(198, 0, 0), RGBc(183, 0, 0), RGBc(169, 0, 0), RGBc(155, 0, 0), RGBc(141, 0, 0) } };


		const ColorPalette Blue_to_Red_moreland = { 33, {RGBc(59, 76, 192), RGBc(68, 90, 204), RGBc(77, 104, 215), RGBc(87, 117, 225), RGBc(98, 130, 234), RGBc(108, 142, 241), RGBc(119, 154, 247), RGBc(130, 165, 251),
														RGBc(141, 176, 254), RGBc(152, 185, 255), RGBc(163, 194, 255), RGBc(174, 201, 253), RGBc(184, 208, 249), RGBc(194, 213, 244), RGBc(204, 217, 238), RGBc(213, 219, 230),
														RGBc(221, 221, 221), RGBc(229, 216, 209), RGBc(236, 211, 197), RGBc(241, 204, 185), RGBc(245, 196, 173), RGBc(247, 187, 160), RGBc(247, 177, 148), RGBc(247, 166, 135),
														RGBc(244, 154, 123), RGBc(241, 141, 111), RGBc(236, 127, 99), RGBc(229, 112, 88), RGBc(222, 96, 77), RGBc(213, 80, 66), RGBc(203, 62, 56), RGBc(192, 40, 47), RGBc(180, 4, 38) } };

		// qualitative (distinctive colors)

		const ColorPalette hard_12 = { 12, {RGBc(166, 206, 227), RGBc(31, 120, 180), RGBc(178, 223, 138), RGBc(51, 160, 44), RGBc(251, 154, 153), RGBc(227, 26, 28), RGBc(253, 191, 111), RGBc(255, 127, 0), RGBc(202, 178, 214), RGBc(106, 61, 154), RGBc(255, 255, 153), RGBc(177, 89, 40) } };
		const ColorPalette soft_12 = { 12, {RGBc(141, 211, 199), RGBc(255, 255, 179), RGBc(190, 186, 218), RGBc(251, 128, 114), RGBc(128, 177, 211), RGBc(253, 180, 98), RGBc(179, 222, 105), RGBc(252, 205, 229), RGBc(217, 217, 217), RGBc(188, 128, 189), RGBc(204, 235, 197), RGBc(255, 237, 111) } };

		const ColorPalette mix_32 = { 32, {RGBc(0xFF, 0x00, 0x00), RGBc(0x00, 0x00, 0xFF), RGBc(0x00, 0xFF, 0x00), RGBc(0x01, 0xFF, 0xFE), RGBc(0xFF, 0xA6, 0xFE), RGBc(0xFF, 0xDB, 0x66), RGBc(0x00, 0x64, 0x01), RGBc(0xFE, 0x89, 0x00),
										  RGBc(0x95, 0x00, 0x3A), RGBc(0x00, 0x7D, 0xB5), RGBc(0x7E, 0x2D, 0xD2), RGBc(0x6A, 0x82, 0x6C), RGBc(0x77, 0x4D, 0x00), RGBc(0x90, 0xFB, 0x92), RGBc(0x01, 0x00, 0x67), RGBc(0xD5, 0xFF, 0x00),
										  RGBc(0xFF, 0x93, 0x7E), RGBc(0xFF, 0xFF, 0x10), RGBc(0xFF, 0x02, 0x9D), RGBc(0x00, 0x76, 0xFF), RGBc(0x7A, 0x47, 0x82), RGBc(0xBD, 0xD3, 0x93), RGBc(0x85, 0xA9, 0x00), RGBc(0xFF, 0x00, 0x56),
										  RGBc(0xA4, 0x24, 0x00), RGBc(0x00, 0xAE, 0x7E), RGBc(0x68, 0x3D, 0x3B), RGBc(0xBD, 0xC6, 0xFF), RGBc(0x26, 0x34, 0x00), RGBc(0xFF, 0x00, 0xF6), RGBc(0x00, 0xB9, 0x17), RGBc(0x00, 0x00, 0x00) } };
										  
		const ColorPalette mix_16 = { 16, {RGBc(230,25,75), RGBc(60,180,75), RGBc(255,225,25), RGBc(0,130,200), RGBc(245,130,48), RGBc(145,30,180), RGBc(70,240,240), RGBc(240,50,230),
										  RGBc(210,245,60), RGBc(250,190,212), RGBc(0,128,128), RGBc(220,190,255), RGBc(170,110,40), RGBc(170,255,195), RGBc(128,128,0), RGBc(255,215,180) } };

	}
}


/* end of file */

