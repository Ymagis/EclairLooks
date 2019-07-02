#include "imageoperator.h"

#include <fstream>

#include <QtCore/QDebug>

#include <utils/generic.h>
#include <utils/chrono.h>
#include <core/imagepipeline.h>
#include "ocio/filetransform.h"


ImageOperator::ImageOperator()
{
    AddParameter<CheckBoxParameter>("Enabled", true);
    AddParameter<SliderParameter>("Opacity", 100.0f, 0.0f, 100.0f, 1.0f);
    AddParameter<SliderParameter>("Contrast", 100.0f, 0.0f, 100.0f, 1.0f);
    AddParameter<SliderParameter>("Color", 100.0f, 0.0f, 100.0f, 1.0);
    // AddParameter<SliderParameter>("Log", 100.0f, 0.0001f, 10000.0f, 1.0f, SliderParameter::Scale::Log);

    using std::placeholders::_1;
    Subscribe<UpdateParam>(std::bind(&ImageOperator::OpUpdateParamCallback, this, _1));
}

ParameterList & ImageOperator::Parameters()
{
    return m_paramList;
}

ParameterList const & ImageOperator::Parameters() const
{
    return m_paramList;
}

std::string ImageOperator::DefaultCategory() const
{
    return m_defaultCategory;
}

ImageOperator::CategoryMapT const & ImageOperator::Categories() const
{
    return m_categoryMap;
}

void ImageOperator::UpdatedParameter(const Parameter &p)
{
    // Order matters here, first give a chance to the operator to update
    // it's internal processing state.
    EmitEvent<Evt::UpdateParam>(p);
    // Then emit the event that will throw a pipeline update...
    EmitEvent<Evt::Update>();
}

bool ImageOperator::IsIdentity() const
{
    auto enabled = m_paramList.Get<CheckBoxParameter>("Enabled")->value();
    return (!enabled || OpIsIdentity());
}

void ImageOperator::Apply(Image & img)
{
    float isolate_cts = m_paramList.Get<SliderParameter>("Contrast")->value() / 100.f;
    float isolate_color = m_paramList.Get<SliderParameter>("Color")->value() / 100.f;

    const Image img_orig = img;

    if (isolate_cts != 1.0f || isolate_color != 1.0f) {
        Image img_apply = img;
        Image img_contrast = img;
        Image img_color = img;
        OpApply(img_apply);

        // Compute 1D LUT, ie. the contrast component of the operator
        uint32_t lut_1d_size = 8192;
        Image img_1d = Image::Ramp1D(lut_1d_size, 0.0f, 1.0f, RampType::NEUTRAL);
        OpApply(img_1d);

        std::string lut_name = "test.spi1d";
        std::ofstream fs(lut_name);
        fs << "Version 1\n";
        fs << "From 0.000000 1.000000\n";
        fs << "Length " << lut_1d_size << "\n";
        fs << "Components 3\n";
        fs << "{\n";

        float *pix = img_1d.pixels_asfloat();
        for (uint64_t i = 0; i < lut_1d_size; ++i) {
            fs << pix[i * 3] << "\t" << pix[i * 3 + 1] << "\t" << pix[i * 3 + 2] << "\n";
        }
        fs << "}\n";
        fs.close();

        // Image with contrast only applied
        OCIOFileTransform file_op;
        file_op.SetFileTransform(lut_name);
        file_op.OpApply(img_contrast);

        // Image with color only applied
        file_op.GetParameter<SelectParameter>("Direction")->setValue("Inverse");
        file_op.OpApply(img_color);
        OpApply(img_color);

        // Mix depending on slider values
        if (isolate_cts == 1.f && isolate_color == 1.f) {
            img = img_apply;
        }
        else if (isolate_cts == 0.f && isolate_color == 0.f) {
            img = img_orig;
        }
        else if (isolate_cts == 0.f || isolate_color == 0.f) {
            float a = std::max(isolate_cts, isolate_color);
            img = img_orig * (1.f - a) + img_contrast * isolate_cts + img_color * isolate_color;
        }
        else if (isolate_cts == 1.f || isolate_color == 1.f) {
            float a = std::min(isolate_cts, isolate_color);
            std::swap(isolate_cts, isolate_color);
            img = img_apply * a + img_contrast * (1.f - isolate_cts) + img_color * (1.f - isolate_color);
        }
        else {
            const Image *source = nullptr;
            float a_contrast = 0.0f;
            float a_color = 0.0f;

            // Work backward from destination image
            if (isolate_cts + isolate_color > 1.f) {
                source = &img_apply;
                a_color = 1.f - isolate_cts;
                a_contrast = 1.f - isolate_color;
            }
            // Work forward from source image
            else {
                source = &img_orig;
                a_color = isolate_color;
                a_contrast = isolate_cts;
            }

            // qInfo() << "Mix : " << a_source;
            // qInfo() << "\tContrast slider" << isolate_cts;
            // qInfo() << "\tColor slider" << isolate_color;
            // qInfo() << "\tSource" << a_source;
            // qInfo() << "\tContrast" << a_contrast;
            // qInfo() << "\tColor" << a_color;
            float a_source = 1.f - (a_color + a_contrast);
            img = *source * a_source + img_contrast * a_contrast + img_color * a_color;
        }
    }
    else {
        OpApply(img);
    }

    float opacity = m_paramList.Get<SliderParameter>("Opacity")->value() / 100.f;
    if(opacity != 1.0) {
        float a = opacity;
        img = img_orig * (1.0f - a) + img * a;
    }
}
