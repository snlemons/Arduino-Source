/*  Color Clustering
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ColorClustering_H
#define PokemonAutomation_CommonFramework_ColorClustering_H

#include "CommonFramework/ImageTypes/RGB32ImageView.h"
#include "FloatPixel.h"

namespace PokemonAutomation{


class PixelEuclideanStatAccumulator{
public:
    void clear();
    void operator+=(FloatPixel pixel);

    uint64_t count() const;
    FloatPixel center() const;
    double deviation() const;

private:
    uint64_t m_count = 0;
    double m_sum_x = 0;
    double m_sum_y = 0;
    double m_sum_z = 0;
    double m_sqr_x = 0;
    double m_sqr_y = 0;
    double m_sqr_z = 0;
};

double cluster_fit_2(
    const ImageViewRGB32& image,
    QRgb color0, PixelEuclideanStatAccumulator& cluster0,
    QRgb color1, PixelEuclideanStatAccumulator& cluster1
);

bool cluster_fit_2(
    const ImageViewRGB32& image,
    QRgb color0, double ratio0,
    QRgb color1, double ratio1,
    double ratio_threshold = 0.2,
    double deviation_threshold = 50,
    double distance_threshold = 0.2
);


}
#endif
