#ifndef BLOCK_H
#define BLOCK_H

#include <QWidget>

class Block : public QWidget
{
    Q_OBJECT
public:
    explicit Block(QWidget *parent = nullptr);

    enum Type{
        type1,
        type2,
        type3,
        type4,
        type5,
        type6,
        invalid
    };

    bool isHighlighted = false , isSelected = false;

signals:
};

#endif // BLOCK_H
