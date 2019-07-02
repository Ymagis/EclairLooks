#include "imagepipeline.h"

#include <iterator>
#include <fstream>
#include <iomanip>

#include <QtCore/QDebug>

#include <utils/generic.h>
#include <utils/chrono.h>


ImagePipeline::ImagePipeline()
{

}

void ImagePipeline::SetInput(const Image & img)
{
    if (!img)
        return;

    m_inputImg = img;
    m_outputImg = img;

    EmitEvent<Evt::NewInput>(m_inputImg);

    Compute();
}

Image & ImagePipeline::GetInput()
{
    return m_inputImg;
}

Image & ImagePipeline::GetOutput()
{
    return m_outputImg;
}

void ImagePipeline::SetName(const std::string &name)
{
    m_name = name;
}

uint8_t ImagePipeline::OperatorCount() const
{
    return m_operators.size();
}

ImageOperator &ImagePipeline::GetOperator(uint8_t index)
{
    return *m_operators[index];
}

ImageOperator* ImagePipeline::AddOperator(ImageOperator * op, int8_t index)
{
    auto pos = m_operators.end();
    if (index >= 0) {
        pos = m_operators.begin();
        std::advance(pos, index);
    }

    UPtr<ImageOperator> & optr = *m_operators.emplace(pos, UPtr<ImageOperator>(op));
    optr->Subscribe<ImageOperator::Update>(std::bind(&ImagePipeline::Compute, this) );

    Compute();

    return optr.get();
}

ImageOperator* ImagePipeline::ReplaceOperator(ImageOperator * op, int8_t index)
{
    if (index >= m_operators.size()) {
        qWarning() << "Cannot replace operator at index" << index;
        return nullptr;
    }

    m_operators[index] = UPtr<ImageOperator>(op);
    m_operators[index]->Subscribe<ImageOperator::Update>(std::bind(&ImagePipeline::Compute, this) );

    return m_operators[index].get();
}

int8_t ImagePipeline::FindOperator(const ImageOperator * op)
{
    for (int i = 0; i < m_operators.size(); ++i)
        if (m_operators[i].get() == op)
            return i;

    return -1;
}

bool ImagePipeline::DeleteOperator(uint8_t index)
{
    if (index >= m_operators.size()) {
        qWarning() << "Cannot remove operator at index" << index;
        return false;
    }

    m_operators.erase(m_operators.begin() + index);
    Compute();

    return true;
}

void ImagePipeline::Reset()
{
    m_operators.clear();

    Compute();
}

void ImagePipeline::Init()
{
    EmitEvent<Evt::NewInput>(m_inputImg);
    EmitEvent<Evt::Update>(m_outputImg);
}

void ImagePipeline::Compute()
{
    Chrono c;
    c.start();

    if (!m_inputImg)
        return;

    m_outputImg = m_inputImg;
    ComputeImage(m_outputImg);

    EmitEvent<Evt::Update>(m_outputImg);
}

void ImagePipeline::ComputeImage(Image & img)
{
    Chrono c;
    c.start();

    for (auto & t : m_operators)
        if (!t->IsIdentity())
            t->Apply(img);

    qInfo() << "Compute (" << QString::fromStdString(m_name)
            << ") Pipeline in : " << fixed << qSetRealNumberPrecision(2)
            << c.ellapsed(Chrono::MILLISECONDS) << "msec.\n";
}

void ImagePipeline::ExportLUT(const std::string & filename, uint32_t size)
{
    // Lattice image
    Image lattice = Image::Lattice(size);

    // Run pipeline
    ComputeImage(lattice);

    // Extract lattice image to lut
    std::ofstream ofs(filename);
    ofs << "LUT_3D_SIZE " << size << "\n";
    ofs << "DOMAIN_MIN 0.000000 0.000000 0.000000\n";
    ofs << "DOMAIN_MAX 1.000000 1.000000 1.000000\n";
    ofs << "\n";

    uint32_t i = 0;
    float * pix = lattice.pixels_asfloat();
    for (uint32_t r = 0; r < size; ++r)
        for (uint32_t g = 0; g < size; ++g)
            for (uint32_t b = 0; b < size; ++b) {
                ofs << std::setprecision(6) << std::fixed << pix[i * 3] << " " << pix[i * 3 + 1] << " " << pix[i * 3 + 2] << "\n";
                i++;
            }
}
