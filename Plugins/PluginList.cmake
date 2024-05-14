
# Plug-ins must be ordered according to their dependencies

set(MITK_PLUGINS

  org.blueberry.core.runtime:ON
  org.blueberry.core.expressions:ON
  org.blueberry.core.commands:ON
  org.blueberry.core.jobs:ON
  org.blueberry.ui.qt:ON
  org.blueberry.ui.qt.help:ON
  org.blueberry.ui.qt.log:ON
  org.blueberry.ui.qt.objectinspector:ON
  org.mitk.core.services:ON
  org.mitk.gui.common:ON
  org.mitk.planarfigure:ON
  org.mitk.core.jobs:ON
  org.mitk.gui.qt.application:ON
  org.mitk.gui.qt.ext:ON
  org.mitk.gui.qt.extapplication:ON
  org.mitk.gui.qt.mitkworkbench.intro:ON
  org.mitk.gui.qt.common:ON
  org.mitk.gui.qt.stdmultiwidgeteditor:ON
  org.mitk.gui.qt.mxnmultiwidgeteditor:ON
  org.mitk.gui.qt.cmdlinemodules:OFF
  org.mitk.gui.qt.chartExample:ON
  org.mitk.gui.qt.datamanager:ON
  org.mitk.gui.qt.datamanagerlight:ON
  org.mitk.gui.qt.datastorageviewertest:ON
  org.mitk.gui.qt.properties:ON
  org.mitk.gui.qt.basicimageprocessing:ON
  org.mitk.gui.qt.dicombrowser:ON
  org.mitk.gui.qt.dicominspector:ON
  org.mitk.gui.qt.dosevisualization:ON
  org.mitk.gui.qt.geometrytools:ON
  org.mitk.gui.qt.igtexamples:OFF
  org.mitk.gui.qt.igttracking:OFF
  org.mitk.gui.qt.openigtlink:OFF
  org.mitk.gui.qt.imagecropper:OFF
  org.mitk.gui.qt.imagenavigator:ON
  org.mitk.gui.qt.viewnavigator:ON
  org.mitk.gui.qt.materialeditor:ON
  org.mitk.gui.qt.measurementtoolbox:ON
  org.mitk.gui.qt.moviemaker:OFF
  org.mitk.gui.qt.pointsetinteraction:ON
  org.mitk.gui.qt.pointsetinteractionmultispectrum:OFF
  org.mitk.gui.qt.python:OFF
  org.mitk.gui.qt.remeshing:OFF
  org.mitk.gui.qt.segmentation:OFF
  org.mitk.gui.qt.deformableclippingplane:OFF
  org.mitk.gui.qt.aicpregistration:OFF
  org.mitk.gui.qt.renderwindowmanager:ON
  org.mitk.gui.qt.semanticrelations:OFF
  org.mitk.gui.qt.toftutorial:OFF
  org.mitk.gui.qt.tofutil:OFF
  org.mitk.gui.qt.tubegraph:OFF
  org.mitk.gui.qt.ugvisualization:OFF
  org.mitk.gui.qt.ultrasound:OFF
  org.mitk.gui.qt.volumevisualization:ON
  org.mitk.gui.qt.eventrecorder:OFF
  org.mitk.gui.qt.xnat:OFF
  org.mitk.gui.qt.igt.app.ultrasoundtrackingnavigation:OFF
  org.mitk.gui.qt.classificationsegmentation:OFF
  org.mitk.gui.qt.overlaymanager:ON
  org.mitk.gui.qt.igt.app.hummelprotocolmeasurements:OFF
  org.mitk.matchpoint.core.helper:OFF
  org.mitk.gui.qt.matchpoint.algorithm.browser:OFF
  org.mitk.gui.qt.matchpoint.algorithm.control:OFF
  org.mitk.gui.qt.matchpoint.mapper:OFF
  org.mitk.gui.qt.matchpoint.framereg:OFF
  org.mitk.gui.qt.matchpoint.visualizer:OFF
  org.mitk.gui.qt.matchpoint.evaluator:OFF
  org.mitk.gui.qt.matchpoint.manipulator:OFF
  org.mitk.gui.qt.preprocessing.resampling:ON
  org.mitk.gui.qt.radiomics:OFF
  org.mitk.gui.qt.cest:OFF
  org.mitk.gui.qt.fit.demo:OFF
  org.mitk.gui.qt.fit.inspector:OFF
  org.mitk.gui.qt.fit.genericfitting:OFF
  org.mitk.gui.qt.pharmacokinetics.mri:OFF
  org.mitk.gui.qt.pharmacokinetics.pet:OFF
  org.mitk.gui.qt.pharmacokinetics.simulation:OFF
  org.mitk.gui.qt.pharmacokinetics.curvedescriptor:OFF
  org.mitk.gui.qt.pharmacokinetics.concentration.mri:OFF
  org.mitk.gui.qt.flowapplication:OFF
  org.mitk.gui.qt.flow.segmentation:OFF
  org.mitk.gui.qt.pixelvalue:ON
  org.mitk.gui.qt.exampleplugin:ON
  org.mitk.lancet.dentalaccuracy:ON
  org.mitk.ch.zhaw.gemapplication:ON
  # org.mitk.ch.zhaw.graphcut:ON
  # org.mitk.ch.zhaw.materialmapping:ON
  org.mitk.ch.zhaw.padding:ON
  # org.mitk.ch.zhaw.resample:ON
  # org.mitk.ch.zhaw.ugvisualization:ON
  # org.mitk.ch.zhaw.volumemesh:ON
  # org.mitk.ch.zhaw.voxel2mesh:ON
  # org.mitk.gui.qt.common.legacy:ON
  #plugins show begin with org.mitk.
  org.mitk.panorama:ON
  org.mitk.upf.rwSegmentationPlugin:ON
)
