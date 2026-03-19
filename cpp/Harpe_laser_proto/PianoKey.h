#ifndef PIANOKEY_H
#define PIANOKEY_H

#endif // PIANOKEY_H

#pragma once
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QObject>

class PianoKey : public QObject, public QGraphicsRectItem
{
    Q_OBJECT  // required for signals

public:
    enum class Type { White, Black , Pressed};

    PianoKey(const QRectF& rect, Type type, int noteIndex, QGraphicsItem* parent = nullptr)
        : QObject(nullptr), QGraphicsRectItem(rect, parent),
        m_type(type), m_noteIndex(noteIndex)
    {
        m_defaultColor = (type == Type::White) ? Qt::white : Qt::black;
        setBrush(QBrush(m_defaultColor));
        setPen(Qt::NoPen);
        setAcceptHoverEvents(true);
        m_keyPressed = 0;
    }

    int get_state()
    {
        return m_keyPressed;
    }


protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
    {
        if (isTopItem(event->scenePos()))
            setBrush(QBrush(Qt::cyan));
        QGraphicsRectItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
    {
        setBrush(QBrush(m_defaultColor));  // restore
        if(m_keyPressed)
            setBrush(QBrush(Qt::blue));
        QGraphicsRectItem::hoverLeaveEvent(event);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (isTopItem(event->scenePos()))
        {
            setBrush(QBrush(Qt::blue));
            if(m_keyPressed)
                setBrush(QBrush(Qt::cyan));
        }
        QGraphicsRectItem::mousePressEvent(event);
        m_keyPressed = !m_keyPressed;
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
    {
        setBrush(QBrush(Qt::cyan));   // cursor still on key hover color
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

private:
    QColor  m_defaultColor;
    Type    m_type;
    int     m_noteIndex;
    int     m_keyPressed;

    bool isTopItem(const QPointF& scenePos) const
    {
        // itemAt() returns the topmost item at that position
        return scene()->itemAt(scenePos, QTransform()) == this;
    }
};