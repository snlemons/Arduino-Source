/*  Common Framework Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Inference/BlackBorderDetector.h"
#include "CommonFramework_Tests.h"
#include "TestUtils.h"


#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

int test_CommonFramework_BlackBorderDetector(const ImageViewRGB32& image, bool target){
    BlackBorderDetector detector;

    bool result = detector.detect(image);

    TEST_RESULT_EQUAL(result, target);

    return 0;
}


}
