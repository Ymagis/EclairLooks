#pragma once

#include <QtWidgets/QListWidget>


class DevWidget;
class ImagePipeline;
class ImageOperator;

// This is a representation of the image processing pipeline
// TODO : In the future, this must be dynamically updated when the
// observed pipeline gets updated, and not statically as of
// today. This will then only be associated with a pipeline object
// and not DevWidget and will be usable from everywhere !
class PipelineWidget : public QListWidget
{
  public:
    PipelineWidget(QWidget *parent = nullptr);

  public:
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  public:
    void setPipeline(ImagePipeline *p);
    void setDevWidget(DevWidget *w);

    void addOperator(ImageOperator *op);
    void addFromFile(const std::string &path);
    void addFromName(const std::string &name);

  private:
    void addOperator(ImageOperator &op);

    void updateSelection(QListWidgetItem * item);
    void disableSelection(int selectedRow);
    void removeSelection(int selectedRow);

  private:
    ImagePipeline *m_pipeline = nullptr;
    DevWidget *m_devWidget = nullptr;
};