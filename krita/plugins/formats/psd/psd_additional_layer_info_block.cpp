#include "psd_additional_layer_info_block.h"

PsdAdditionalLayerInfoBlock::PsdAdditionalLayerInfoBlock()
{
}

bool PsdAdditionalLayerInfoBlock::read(QIODevice *io)
{

    if (key == "SoCo") {

    }
    else if (key == "GdFl") {

    }
    else if (key == "PtFl") {

    }
    else if (key == "brit") {

    }
    else if (key == "levl") {

    }
    else if (key == "curv") {

    }
    else if (key == "expA") {

    }
    else if (key == "vibA") {

    }
    else if (key == "hue") {

    }
    else if (key == "hue2") {

    }
    else if (key == "blnc") {

    }
    else if (key == "blwh") {

    }
    else if (key == "phfl") {

    }
    else if (key == "mixr") {

    }
    else if (key == "clrL") {

    }
    else if (key == "nvrt") {

    }
    else if (key == "post") {

    }
    else if (key == "thrs") {

    }
    else if (key == "grdm") {

    }
    else if (key == "selc") {

    }
    else if (key == "lrFX") {

    }
    else if (key == "tySh") {

    }
    else if (key == "luni") {

    }
    else if (key == "lyid") {

    }
    else if (key == "lfx2") {

    }
    else if (key == "Patt" || key == "Pat2" || key == "Pat3") {

    }
    else if (key == "Anno") {

    }
    else if (key == "clbl") {

    }
    else if (key == "infx") {

    }
    else if (key == "knko") {

    }
    else if (key == "spf") {

    }
    else if (key == "lclr") {

    }
    else if (key == "fxrp") {

    }
    else if (key == "grdm") {

    }
    else if (key == "lsct") {

    }
    else if (key == "brst") {

    }
    else if (key == "SoCo") {

    }
    else if (key == "PtFl") {

    }
    else if (key == "GdFl") {

    }
    else if (key == "vmsk" || key == "vsms") { // If key is 'vsms' then we are writing for (Photoshop CS6) and the document will have a 'vscg' key

    }
    else if (key == "TySh") {

    }
    else if (key == "ffxi") {

    }
    else if (key == "lnsr") {

    }
    else if (key == "shpa") {

    }
    else if (key == "shmd") {

    }
    else if (key == "lyvr") {

    }
    else if (key == "tsly") {

    }
    else if (key == "lmgm") {

    }
    else if (key == "vmgm") {

    }
    else if (key == "plLd") { // Replaced by SoLd in CS3

    }
    else if (key == "linkD" || key == "lnk2" || key == "lnk3") {

    }
    else if (key == "phfl") {

    }
    else if (key == "blwh") {

    }
    else if (key == "CgEd") {

    }
    else if (key == "Txt2") {

    }
    else if (key == "vibA") {

    }
    else if (key == "pths") {

    }
    else if (key == "anFX") {

    }
    else if (key == "FMsk") {

    }
    else if (key == "SoLd") {

    }
    else if (key == "vstk") {

    }
    else if (key == "vsCg") {

    }
    else if (key == "sn2P") {

    }
    else if (key == "vogk") {

    }
    else if (key == "Mtrn" || key == "Mt16" || key == "Mt32") { // There is no data associated with these keys.

    }
    else if (key == "LMsk") {

    }
    else if (key == "expA") {

    }
    else if (key == "FXid") {

    }
    else if (key == "FEid") {

    }




    return true;


}

bool PsdAdditionalLayerInfoBlock::write(QIODevice *io, KisNodeSP node)
{
    return true;
}


bool PsdAdditionalLayerInfoBlock::valid()
{

    return true;
}
