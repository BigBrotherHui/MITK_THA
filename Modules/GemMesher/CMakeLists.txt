mitk_create_module(
        INCLUDE_DIRS src/lib/tetgen1.5.0
        DEPENDS MitkCore
        PACKAGE_DEPENDS PUBLIC CGAL|Core+ImageIO)
