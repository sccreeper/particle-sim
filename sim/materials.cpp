#include "materials.hpp"

namespace mat
{

    State decideState(const Material &m, float temp)
    {

        if (temp >= m.boilingPoint)
        {
            return Gas;
        }
        else if (temp >= m.meltingPoint)
        {
            return Liquid;
        }
        else
        {
            return Solid;
        }
    };

} // namespace mat