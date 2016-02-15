This directory and file

initial_with_selection_with_shape_clone1_paintDevice.png

exists only for the sake of KisNodeManagerTest::testConvertToPaintLayer(),
because after conversion a clone layer gets a paint device. In other
circumstances the paintDevice() of a clone layer is null.
