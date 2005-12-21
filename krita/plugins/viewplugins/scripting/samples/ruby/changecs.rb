require "krosskritacore"

doc = Krosskritacore::get("KritaDocument")
image = doc.getImage()
image.convertToColorspace("LABA")
