project(org_mitk_upf_rwSegmentationPlugin)
mitk_create_plugin(
  EXPORT_DIRECTIVE RWSEGMENTATIONPLUGIN_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS PUBLIC Eigen
)