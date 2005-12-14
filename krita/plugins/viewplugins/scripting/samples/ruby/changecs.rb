require "krosskritacore"

doc = $krosskritacore.get("KritaDocument")
image = doc.getImage()
image.convertToColorspace("LABA")
