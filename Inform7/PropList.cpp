#include "stdafx.h"
#include "PropList.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool PropList::Load(LPCSTR path)
{
  // Open the XML file
  CFile xmlFile;
  if (xmlFile.Open(path,CFile::modeRead) == FALSE)
    return false;

  // Read in the XML file
  int xmlSize = (int)xmlFile.GetLength();
  CString xml;
  LPSTR xmlBuffer = xml.GetBufferSetLength(xmlSize);
  xmlFile.Read(xmlBuffer,xmlSize);
  xml.ReleaseBuffer(xmlSize);

  // Make sure that the DTD is loaded from the local file
  CString file;
  file.Format("file://%s\\Web\\",theApp.GetAppDir());
  xml.Replace("http://",file);

  // Create an XML document instance
  if (m_doc == NULL)
  {
    if (FAILED(m_doc.CoCreateInstance(CLSID_DOMDocument)))
      return false;
  }

  // Load the settings XML into the document
  VARIANT_BOOL success = 0;
  CStreamOnCString xmlStream(xml);
  if (m_doc->load(CComVariant(&xmlStream),&success) != S_OK)
  {
    CComPtr<IXMLDOMParseError> error;
    m_doc->get_parseError(&error);
    CComBSTR reason;
    error->get_reason(&reason);
    TRACE(reason.m_str);
    return false;
  }
  return true;
}

CStringW PropList::GetString(LPCWSTR dict1, LPCWSTR dict2)
{
  CComPtr<IXMLDOMNode> node = GetNode(dict1,dict2);
  if (node != NULL)
  {
    CComBSTR text;
    if (SUCCEEDED(node->get_text(&text)))
      return text.m_str;
  }
  return L"";
}

int PropList::GetNumber(LPCWSTR dict1, LPCWSTR dict2)
{
  CComPtr<IXMLDOMNode> node = GetNode(dict1,dict2);
  if (node != NULL)
  {
    CComBSTR text;
    if (SUCCEEDED(node->get_text(&text)))
      return _wtoi(text.m_str);
  }
  return 0;
}

bool PropList::GetBoolean(LPCWSTR dict1, LPCWSTR dict2, bool defaultValue)
{
  CComPtr<IXMLDOMNode> node = GetNode(dict1,dict2);
  if (node != NULL)
  {
    CComBSTR name;
    if (SUCCEEDED(node->get_nodeName(&name)))
      return (name == L"true");
  }
  return defaultValue;
}

void PropList::GetKeyValues(LPCWSTR dict, KeyValueMap& values)
{
  if (m_doc == NULL)
    return;

  CStringW query;
  CComPtr<IXMLDOMNode> key;

  query.Format(L"/plist/dict/key[.='%s']",dict);
  if (m_doc->selectSingleNode(CComBSTR(query),&key) == S_OK)
  {
    CComPtr<IXMLDOMNode> value;
    CComPtr<IXMLDOMNodeList> subkeys;

    key->get_nextSibling(&value);
    if (value->selectNodes(CComBSTR("key"),&subkeys) == S_OK)
    {
      CComPtr<IXMLDOMNode> subkey;

      while (subkeys->nextNode(&subkey) == S_OK)
      {
        CComPtr<IXMLDOMNode> subvalue;

        subkey->get_nextSibling(&subvalue);

        CComBSTR textKey, textValue;
        if (SUCCEEDED(subkey->get_text(&textKey)))
        {
          if (SUCCEEDED(subvalue->get_text(&textValue)))
            values[textKey] = textValue;
        }
        subkey = NULL;
      }
    }
  }
}

IXMLDOMNode* PropList::GetNode(LPCWSTR dict1, LPCWSTR dict2)
{
  if (m_doc == NULL)
    return NULL;

  CStringW query;
  CComPtr<IXMLDOMNode> key, value;

  query.Format(L"/plist/dict/key[.='%s']",dict1);
  if (m_doc->selectSingleNode(CComBSTR(query),&key) == S_OK)
  {
    key->get_nextSibling(&value);
    key.Release();

    query.Format(L"key[.='%s']",dict2);
    if (value->selectSingleNode(CComBSTR(query),&key) == S_OK)
    {
      value.Release();
      key->get_nextSibling(&value);
      return value.Detach();
    }
  }
  return NULL;
}
