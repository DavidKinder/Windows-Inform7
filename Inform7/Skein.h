#pragma once

#include "Diff.h"

#include <map>
#include <set>
#include <vector>

class Skein
{
public:
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

  void Reset(bool current);
  void Layout(CDC& dc, int spacing, bool force);
  void GetTreeExtent(int& width, int& depth);

  void NewLine(const CStringW& line);
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
    const Diff::DiffResults& GetTranscriptDiffs(void);
    const Diff::DiffResults& GetExpectedDiffs(void);

    bool GetChanged(void);

    enum ExpectedCompare
    {
      ExpectedSame,
      ExpectedNearlySame,
      ExpectedDifferent,
    };
    ExpectedCompare GetDiffers(void);

    bool GetLocked(void);
    bool SetLocked(bool locked);

    void NewTranscriptText(const CStringW& transcript);

    bool Bless(void);
    bool CanBless(void);
    bool SetExpectedText(LPCWSTR text);

    int GetLineWidth(CDC& dc);
    int GetLayoutWidth(void);
    int GetLineTextWidth(void);
    int GetLabelTextWidth(void);
    void ClearWidths(void);

    int GetDepth(void);

    void Add(Node* child);
    bool Remove(Node* child);
    void RemoveAll(void);
    bool RemoveSingle(Node* child);
    void Replace(Node* oldNode, Node* newNode);

    Node* Find(const CStringW& line);
    Node* FindAncestor(Node* descendant);

    const char* GetUniqueId(void);
    void SaveNodes(FILE* skeinFile);

    void GetLabels(std::map<CStringW,Node*>& labels);
    bool HasLabels(void);

    void GetNodesByDepth(int depth, std::vector<std::vector<Node*> >& nodesByDepth);
    int GetX(void);
    void SetX(int x);
    void ShiftX(int shift);

    void AnimatePrepare(int depth);
    void AnimateClear(void);
    int GetAnimateX(int pct);
    int GetAnimateY(int depth, int spacing, int pct);

  private:
    void CompareWithExpected(void);
    void OverwriteBanner(CStringW& inStr);
    CStringW StripWhite(const CStringW& inStr);

    CStringW m_line;
    CStringW m_label;
    CString m_id;

    CStringW m_textTranscript;
    CStringW m_textExpected;
    Diff::DiffResults m_diffTranscript;
    Diff::DiffResults m_diffExpected;

    bool m_locked;
    bool m_changed;
    ExpectedCompare m_differs;

    int m_width;
    int m_lineWidth;
    int m_labelWidth;
    int m_x;

    bool m_anim;
    int m_animX;
    int m_animDepth;

    Node* m_parent;
    CArray<Node*> m_children;
  };

  Node* GetRoot(void);

  Node* GetCurrent(void);
  void SetCurrent(Node* node);
  bool InCurrentThread(Node* node);
  bool InThread(Node* node, Node* endNode);
  Node* GetPlayed(void);

  Node* AddNew(Node* node);
  Node* AddNewParent(Node* node);
  bool RemoveAll(Node* node, bool notify = true);
  bool RemoveSingle(Node* node);
  void SetLine(Node* node, LPCWSTR line);

  void SetLabel(Node* node, LPCWSTR label);
  void GetLabels(std::map<CStringW,Node*>& labels);
  bool HasLabels(void);

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

  void SaveTranscript(Node* node, const char* path);

  enum Change
  {
    TreeChanged,
    ThreadChanged,
    NodeTextChanged,
    NodeColourChanged,
    LockChanged,
    TranscriptThreadChanged,
  };

  enum Show
  {
    JustShow,
    JustSelect,
    ShowSelect,
    ShowNewLine,
    ShowNewTranscript,
  };

  class Listener
  {
  public:
    virtual void SkeinChanged(Change change) = 0;
    virtual void SkeinEdited(bool edited) = 0;
    virtual void SkeinShowNode(Node* node, Show why) = 0;
  };

  void AddListener(Listener* listener);
  void NotifyChange(Change change);
  void NotifyEdit(bool edited);
  void NotifyShowNode(Node* node, Show why);

private:
  static LPCTSTR ToXML_UTF8(bool value);
  static CComBSTR StringFromXML(IXMLDOMNode* node, LPWSTR query);
  static bool BoolFromXML(IXMLDOMNode* node, LPWSTR query, bool ifNon);
  static int IntFromXML(IXMLDOMNode* node, LPWSTR query);

  struct Instance
  {
    CString skeinFile;
    bool edited;
    Node* root;
    Node* current;

    Instance() : edited(false), root(NULL), current(NULL)
    {
    }

    bool Save(const char* path);
  };
  Instance m_inst;
  std::vector<Instance> m_other;

  bool m_layout;
  std::vector<Listener*> m_listeners;
  Node* m_played;
};
