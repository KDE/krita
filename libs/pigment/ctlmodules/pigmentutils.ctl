
namespace PigmentUtils {
    float int16toFloat( int v )
    {
        float vf = v;
        return vf / 65535;
    }
    
    int floatToInt16( float v )
    {
        return v * 65535;
    }
}
