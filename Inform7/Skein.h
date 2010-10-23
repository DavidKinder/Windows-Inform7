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
  void Import(const char* path);
  bool IsEdited(void);
  bool NeedSaveWarn(int& maxTemp);

  void Reset(bool current);
  void Layout(CDC& dc, CFont* labelFont, int spacing);

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
      const CStringW& expected, bool played, bool changed, bool temp, int score);
    ~Node();

    Node* GetParent(void);
    void SetParent(Node* parent);

    int GetNumChildren(void);
    Node* GetChild(int index);

    const CStringW& GetLine(void);
    bool SetLine(LPWSTR line);

    const CStringW& GetLabel(void);
    bool SetLabel(LPWSTR label);
    bool HasLabel(void);

    const CStringW& GetTranscriptText(void);
    const CStringW& GetExpectedText(void);
    const Diff::DiffResults& GetTranscriptDiffs(void);
    const Diff::DiffResults& GetExpectedDiffs(void);

    bool GetChanged(void);
    bool GetTemporary(void);

    enum ExpectedCompare
    {
      ExpectedSame,
      ExpectedNearlySame,
      ExpectedDifferent,
    };
    ExpectedCompare GetDiffers(void);

    void SetPlayed(void);
    bool SetTemporary(bool temp);
    void NewTranscriptText(const CStringW& transcript);

    bool Bless(void);
    bool CanBless(void);
    bool SetExpectedText(LPCWSTR text);

    int GetLineWidth(CDC& dc, CFont* labelFont);
    int GetLineTextWidth(void);
    int GetLabelTextWidth(void);
    int GetTreeWidth(CDC& dc, CFont* labelFont, int spacing);

    int GetDepth(void);
    int GetMaxDepth(void);

    void Add(Node* child);
    bool Remove(Node* child);
    void RemoveAll(void);
    bool RemoveSingle(Node* child);
    void Replace(Node* oldNode, Node* newNode);

    Node* Find(const CStringW& line);
    Node* FindAncestor(Node* descendant);

    const char* GetUniqueId(void);
    void GetTempNodes(std::set<Node*>& nodes, int max);
    bool WillSaveNode(const std::set<Node*>& tempNodes);
    void SaveNodes(FILE* skeinFile, const std::set<Node*>& tempNodes);

    void Layout(CDC& dc, CFont* labelFont, int x, int spacing);
    int GetX(void);

    void GetLabels(std::map<CStringW,Node*>& labels);
    bool HasLabels(void);

  private:
    void CompareWithExpected(void);
    CStringW StripWhite(const CStringW& inStr);

    CStringW m_line;
    CStringW m_label;
    CString m_id;

    CStringW m_textTranscript;
    CStringW m_textExpected;
    Diff::DiffResults m_diffTranscript;
    Diff::DiffResults m_diffExpected;

    bool m_played;
    bool m_changed;
    bool m_temp;
    ExpectedCompare m_differs;
    int m_score;

    int m_width;
    int m_lineWidth;
    int m_labelWidth;
    int m_x;

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
  void SetLine(Node* node, LPWSTR line);
  void SetLabel(Node* node, LPWSTR label);

  void Lock(Node* node);
  void Unlock(Node* node, bool notify = true);
  void Trim(Node* node, bool running, bool notify = true);

  void GetLabels(std::map<CStringW,Node*>& labels);
  bool HasLabels(void);

  void Bless(Node* node, bool all);
  bool CanBless(Node* node, bool all);
  void SetExpectedText(Node* node, LPCWSTR text);

  Node* GetThreadTop(Node* node);
  Node* GetThreadBottom(Node* node);
  bool IsValidNode(Node* testNode, Node* node = NULL);
  int GetBlessedThreadEnds(std::vector<Node*>& nodes, Node* node = NULL);
  Node* GetFirstDifferent(Node* node = NULL);
  void GetAllNodes(CArray<Skein::Node*,Skein::Node*>& nodes, Node* node = NULL);

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

  bool m_layout;
  bool m_edited;
  std::vector<Listener*> m_listeners;

  Node* m_root;
  Node* m_current;
  Node* m_played;

  int m_maxSaveTemp;
};
