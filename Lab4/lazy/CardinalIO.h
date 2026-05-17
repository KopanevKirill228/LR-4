#pragma once

#include "Cardinal.h"

#include <iostream>


inline std::ostream& operator<<(std::ostream& out, const Cardinal& cardinal) {
    if (cardinal.IsInfinite()) {
        out << "Infinity";
    }
    else {
        out << cardinal.GetFiniteValue();
    }

    return out;
}