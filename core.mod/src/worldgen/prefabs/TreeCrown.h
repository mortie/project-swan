#pragma once

#include "../Prefab.h"
#include "tiles.h"
#include <array>

namespace CoreMod::TreeCrown {

inline Prefab::Map map = {
	{'#', tiles::tree__leaves},
	{'L', tiles::tree__leaves__left},
	{'R', tiles::tree__leaves__right},
	{'T', tiles::tree__top},
	{'X', tiles::tree__cross},
	{'|', tiles::tree__center},
	{'(', tiles::tree__branch__left},
	{')', tiles::tree__branch__right},
	{'<', tiles::tree__stub__left},
	{'>', tiles::tree__stub__right},
};

inline Prefab variant1(map, {
	"LTR",
	"<X>",
});

inline Prefab variant2(map, {
	" LTR ",
	"L<X>R",
	"<(X)>",
});

inline Prefab variant3(map, {
	"  LTR  ",
	" L<X>R ",
	"L<(X)>R",
	"<((X))>",
});

inline Prefab variant4(map, {
	"LTR",
	"#|#",
});

inline Prefab variant5(map, {
	" L#T#R ",
	" <(X)> ",
	"L##|##R",
	"<((X))>",
});

inline std::array variants = {
	&variant1,
	&variant2,
	&variant3,
	&variant4,
	&variant5,
};

}
