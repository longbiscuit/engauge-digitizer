#ifndef TUTORIAL_STATE_INTRODUCTION_H
#define TUTORIAL_STATE_INTRODUCTION_H

class QGraphicsPixmapItem;
class QGraphicsTextItem;
class TutorialButton;
class TutorialStateContext;

/// Introduction state/panel is the first panel the user sees
class TutorialStateIntroduction : public TutorialStateAbstractBase
{
 public:
  /// Single constructor
  TutorialStateIntroduction (TutorialStateContext &context);

  virtual void begin ();
  virtual void end ();

 private:

  QGraphicsPixmapItem *m_background;
  QGraphicsTextItem *m_text0;
  QGraphicsTextItem *m_text1;
  QGraphicsTextItem *m_text2;
  TutorialButton *m_next;
};

#endif // TUTORIAL_STATE_INTRODUCTION_H
