#include "pch.h"
#include "winrt/test_component_fast.Composition.h"

using namespace winrt::test_component_fast::Composition;


 //All, yes all, of the QIs in the test below are avoided by the fastabi.
 //That's 8 calls to QueryInterface and 8 calls to Release.


TEST_CASE("scratch")
{
    // IActivationFactory::ActivateInstance is called for the default constructor.
    // Under the slowabi, the ActivateInstance method returns IInspectable so each
    // caller must QI for the default interface.
    Compositor compositor;

    SpriteVisual v1 = compositor.CreateSpriteVisual();
    SpriteVisual v2 = compositor.CreateSpriteVisual();

    // Under the slowabi, this needs a QI for ICompositionObject.
    v1.Compositor();

    // Under the slowabi, this needs a QI for ICompositionObject2.
    v1.StartAnimationGroup();

    // Under the slowabi, this needs a QI for IVisual.
    v1.Offset();

    // Under the slowabi, this needs a QI for IVisual2 to call ParentForTransform
    // *and* the v2 parameter needs a QI for IVisual.
    v1.ParentForTransform(v2);

    // Since Brush is on the default interface of SpriteVisual there's no QI here
    // for both slowabi and fastabi. Literally the only statement in this whole
    // function that doesn't inject at least one QI and Release.
    v1.Brush();

    // Under the slowabi, this needs a QI for ISpriteVisual2.
    v1.Shadow();
}
