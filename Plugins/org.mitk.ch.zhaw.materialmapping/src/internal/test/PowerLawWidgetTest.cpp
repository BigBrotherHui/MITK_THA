#include "catch.hpp"

#include "../PowerLawWidget.h"
#include "../PowerLawWidgetManager.h"

TEST_CASE("PowerLawWidget"){
    PowerLawWidget w0, w1;
    w0.setFactor(1);
    w0.setExponent(2);
    w0.setOffset(3);

    w1.setFactor(4);
    w1.setExponent(5);
    w1.setOffset(6);

    SECTION("constructing parameters from gui"){
        PowerLawParameters expectedP0(1,2,3);
        PowerLawParameters expectedP1(4,5,6);
        auto p0 = w0.getPowerLawParameters();
        auto p1 = w1.getPowerLawParameters();
        REQUIRE(p0==expectedP0);
        REQUIRE(p1==expectedP1);
    }

    SECTION("signals between 2 widgets"){
        w0.connect(&w1);
        w0.setMax(50);
        REQUIRE(w0.getMax() == w1.getMin());
        w1.setMax(75);
        REQUIRE(w0.getMax() == w1.getMin());
    }
}

TEST_CASE("PowerLawWidgetManager"){
    auto parent = new QWidget;
    auto layout = new QHBoxLayout;
    parent->setLayout(layout);

    PowerLawWidgetManager manager(parent);

    SECTION("check signals"){
        auto w0 = manager.addPowerLaw();
        auto w1 = manager.addPowerLaw();

        w0->setMax(50);
        REQUIRE(w0->getMax() == w1->getMin());
        w1->setMax(75);
        REQUIRE(w0->getMax() == w1->getMin());
    }

    SECTION("add / remove"){
        REQUIRE(manager.getNumberOfWidgets() == 2); // default value
        REQUIRE(manager.removePowerLaw() == true); // can remove the default
        REQUIRE(manager.getNumberOfWidgets() == 1);
        REQUIRE(manager.removePowerLaw() == false); // can't go below 1 row
        REQUIRE(manager.getNumberOfWidgets() == 1);

        REQUIRE(manager.addPowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 2);

        REQUIRE(manager.addPowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 3);

        REQUIRE(manager.addPowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 4);

        REQUIRE(manager.removePowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 3);

        REQUIRE(manager.removePowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 2);

        REQUIRE(manager.removePowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 1);

        REQUIRE(manager.removePowerLaw() == false); // cant remove the last widget
        REQUIRE(manager.getNumberOfWidgets() == 1);

        REQUIRE(manager.addPowerLaw());
        REQUIRE(manager.getNumberOfWidgets() == 2);
    }

    SECTION("range adaption on add / remove"){
        REQUIRE(manager.removePowerLaw() == true); // pop the default value
        REQUIRE(manager.getNumberOfWidgets() == 1);

        // TODO: these should be a field in the widgets
        auto minValue = PowerLawWidget::MinValue;
        auto maxValue = PowerLawWidget::MaxValue;

        auto w0 = manager.getWidget(0);
        REQUIRE(w0->getMin() == minValue);
        REQUIRE(w0->getMax() == minValue); // note that this is the widgets max which is represented as a Qt special value = min

        auto w1 = manager.addPowerLaw();
        REQUIRE(w0->getMin() == minValue);
        REQUIRE(w0->getMax() == w1->getMin());
        REQUIRE(w0->getMax() != minValue);
        REQUIRE(w1->getMax() == minValue);

        auto w2 = manager.addPowerLaw();
        REQUIRE(w0->getMin() == minValue);
        REQUIRE(w0->getMax() != minValue);
        REQUIRE(w0->getMax() == w1->getMin());
        REQUIRE(w1->getMax() == w2->getMin());
        REQUIRE(w2->getMax() == minValue);

        auto w3 = manager.addPowerLaw();
        REQUIRE(w2->getMax() == w3->getMin());
        REQUIRE(w3->getMax() == minValue);

        REQUIRE(manager.removePowerLaw());
        REQUIRE(w1->getMax() == w2->getMin());
        REQUIRE(w2->getMax() == minValue);

        REQUIRE(manager.removePowerLaw());
        REQUIRE(w0->getMax() == w1->getMin());
        REQUIRE(w1->getMax() == minValue);

        REQUIRE(manager.removePowerLaw());
        REQUIRE(w0->getMax() == minValue);
        REQUIRE(w0->getMax() == minValue);

        REQUIRE(manager.removePowerLaw() == false); // can' remove the last one
    }
}