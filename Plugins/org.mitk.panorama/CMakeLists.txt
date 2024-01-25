project(org_mitk_panorama)

mitk_create_plugin(
    EXPORT_DIRECTIVE Panorama_EXPORT
	EXPORTED_INCLUDE_SUFFIXES src
	MODULE_DEPENDS MitkQtWidgetsExt MitkModelFit MitkModelFitUI
	PACKAGE_DEPENDS PRIVATE ITK|IOGDCM+IONIFTI+IOPNG+Smoothing+LabelMap VTK|ImagingGeneral
)
#MitkTestingHelper 
