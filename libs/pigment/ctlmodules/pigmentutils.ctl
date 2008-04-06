
namespace PigmentUtils {
    float int16toFloat( int v )
    {
        float vf = v;
        return vf / 65535;
    }
    
    int floatToInt16( float v )
    {
        if( v < 0.0 ) return 0;
        if( v > 1.0 ) return 65535;
        return v * 65535;
    }
}
