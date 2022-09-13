#pragma once

#include "TranscriptDiff.h"

#include <map>
#include <vector>

#define LAYOUTS 2

class TranscriptPane;

class Skein
{
public:
  class Node;

  Skein();
  ~Skein();

  void Load(const char* path);
  bool Save(const char* path);
  void Reset(void);
  void Import(const char* path);
  bool IsActive(void);
  bool IsEdited(void);

  void SetFile(const char* fileName);
  bool ChangeFile(const char* fileName, const char* path);

  void Reset(bool playTo);
  void InvalidateLayout(void);

  enum LayoutMode
  {
    LayoutDefault,
    LayoutReposition,
    LayoutRecalculate
  };

  void Layout(CDC& dc, int idx, LayoutMode mode,
    const CSize& spacing, TranscriptPane& transcript);
  CSize GetTreeExtent(int idx);

  void NewLine(const CStringW& line, bool test);
  bool NextLine(CStringW& line);
  void UpdateAfterPlaying(const CStringW& transcript);

  enum EscapeAction
  {
    UsePrintable,
    UseCharacters,
    RemoveEscapes,
  };
  CStringW EscapeLine(const CStringW& line, EscapeAction action);

  bool GetLineFromHistory(CStringW& line, int history);

  class Node
  {
  public:
    Node(const CStringW& line, const CStringW& label, const CStringW& transcript,
      const CStringW& expected, bool changed);
    ~Node();

    Node* GetParent(void);
    void SetParent(Node* parent);

    int GetNumChildren(void);
    Node* GetChild(int index);

    const CStringW& GetLine(void);
    bool SetLine(LPCWSTR line);

    const CStringW& GetLabel(void);
    bool SetLabel(LPCWSTR label);
    bool HasLabel(void);

    const CStringW& GetTranscriptText(void);
    const CStringW& GetExpectedText(void);
    const TranscriptDiff& GetTranscriptDiff(void);

    bool GetChanged(void);
    bool GetDiffers(void);

    bool GetLocked(void);
    bool SetLocked(bool locked);

    bool IsTestCommand(void);
    bool IsTestSubItem(void);
    void SetTestSubItem(void);
    int GetNumTestSubChildren(void);
    void NewTranscriptText(LPCWSTR text);

    bool Bless(void);
    bool SetExpectedText(LPCWSTR text);

    int CalcLineWidth(CDC& dc, int idx);
    int GetLayoutWidth(int idx);
    int GetLineTextWidth(int idx);
    int GetLabelTextWidth(int idx);
    void ClearWidths(int idx);
    int GetLeftmostAfterX(int idx, int x);

    void Add(Node* child);
    bool Remove(Node* child);
    void RemoveAll(void);
    void RemoveAllExcept(Node* keep);
    bool RemoveSingle(Node* child);
    void Replace(Node* oldNode, Node* newNode);
    bool SortChildren(void);

    Node* Find(const CStringW& line);
    Node* FindAncestor(Node* descendant);

    const char* GetUniqueId(void);
    void SaveNodes(FILE* skeinFile);

    void GetNodesByDepth(int depth, std::vector<std::vector<Node*> >& nodesByDepth);
    int GetX(int idx);
    void SetX(int idx, int x);
    void ShiftX(int idx, int shift);
    int GetY(int idx);
    void SetY(int idx, int y);
    void ShiftY(int idx, int shift);

    void AnimatePrepare(int idx);
    void AnimateClear(void);
    CPoint GetAnimatePos(int idx, int pct);
    bool IsAnimated(int idx);

  private:
    void CompareWithExpected(void);

    CStringW m_line;
    CStringW m_label;
    CString m_id;

    CStringW m_textTranscript;
    CStringW m_textExpected;
    TranscriptDiff m_diff;

    bool m_locked;
    bool m_changed;
    bool m_testSubItem;

    Node* m_parent;
    CArray<Node*> m_children;

    struct LayoutInfo
    {
      int width;
      int lineWidth;
      int labelWidth;
      CPoint pos;

      bool anim;
      CPoint animPos;

      LayoutInfo();
      void ClearWidths();
    };
    LayoutInfo m_layout[LAYOUTS];
  };

  Node* GetRoot(void);

  Node* GetPlayTo(void);
  void SetPlayTo(Node* node);
  bool InPlayThread(Node* node);
  bool InThread(Node* node, Node* endNode);
  Node* GetPlayed(void);

  Node* AddNew(Node* node);
  Node* AddNewParent(Node* node);
  bool RemoveAll(Node* node, bool notify = true);
  bool RemoveSingle(Node* node);
  void SortSiblings(Node* node);
  void SetLine(Node* node, LPCWSTR line);
  void SetLabel(Node* node, LPCWSTR label);

  void Lock(Node* node);
  void Unlock(Node* node, bool notify = true);
  void Trim(Node* node, bool running, bool notify = true);

  void Bless(Node* node, bool all);
  bool CanBless(Node* node, bool all);
  void SetExpectedText(Node* node, LPCWSTR text);

  Node* GetThreadEnd(Node* node);
  bool IsValidNode(Node* testNode, Node* node = NULL);
  void GetThreadEnds(std::vector<Node*>& nodes, Node* node = NULL);
  Node* GetFirstDifferent(Node* node = NULL);
  void GetAllNodes(CArray<Skein::Node*,Skein::Node*>& nodes, Node* node = NULL);
  Node* FindNode(const char* id, Node* node = NULL);

  enum Change
  {
    TreeChanged,
    PlayedChanged,
    NodeTextChanged,
    NodeTranscriptChanged,
    LockChanged
  };

  class Listener
  {
  public:
    virtual void SkeinChanged(Change change) = 0;
    virtual void SkeinEdited(bool edited) = 0;
    virtual void SkeinShowNode(Node* node, bool select) = 0;
  };

  void AddListener(Listener* listener);
  void NotifyChange(Change change);
  void NotifyEdit(bool edited);
  void NotifyShowNode(Node* node);

private:
  static bool IsTestCommand(const CStringW& line);
  static void SeparateByBracketedSequentialNumbers(const CStringW& text, std::vector<CStringW>& results);
  static CStringW CommandForTestingEntry(const CStringW& entry);
  static CStringW OutputForTestingEntry(const CStringW& entry);

  static LPCTSTR ToXML_UTF8(bool value);
  static CComBSTR StringFromXML(IXMLDOMNode* node, LPWSTR query);
  static bool BoolFromXML(IXMLDOMNode* node, LPWSTR query, bool ifNon);
  static int IntFromXML(IXMLDOMNode* node, LPWSTR query);

  struct Instance
  {
    CString skeinFile;
    bool edited;
    Node* root;

    Instance() : edited(false), root(NULL)
    {
    }

    bool Save(const char* path);
  };
  Instance m_inst;
  std::vector<Instance> m_other;

  bool m_laidOut[LAYOUTS];
  std::vector<Listener*> m_listeners;

  // The node to play down to
  Node* m_playTo;
  // The last node that was played
  Node* m_played;
};
