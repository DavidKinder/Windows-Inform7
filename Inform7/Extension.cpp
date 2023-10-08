#include "stdafx.h"
#include "Extension.h"

#include "Inform.h"
#include "TextFormat.h"

#include "unzip.h" // zlib minizip
#include "nlohmann/json.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Extension::Location::Location(const char* a, const char* t, const char* p)
  : author(a), title(t), path(p)
{
}

bool Extension::Location::operator<(const Location& el) const
{
  if (author != el.author)
    return author < el.author;
  return title < el.title;
}

void Extension::GetAll(Set& extensions, const char* materialsDir)
{
  // Get legacy folder extensions
  for (const auto& extension : Legacy::Get())
    extensions.insert(extension);

  // Get built in extensions
  CString path;
  path.Format("%s\\Internal\\Extensions",(LPCSTR)theApp.GetAppDir());
  Find(path,extensions);

  // Get project extensions
  path.Format("%s\\Extensions",materialsDir);
  Find(path,extensions);
}

void Extension::Find(const char* dir, Set& extensions)
{
  CString pattern;
  pattern.Format("%s\\*.*",dir);
  CFileFind findAuthor;
  BOOL foundAuthor = findAuthor.FindFile(pattern);
  while (foundAuthor)
  {
    foundAuthor = findAuthor.FindNextFile();
    if (findAuthor.IsDirectory() && (findAuthor.GetFileName() != "Reserved"))
    {
      // Find single file extensions
      CFileFind findExt;
      BOOL foundExt = findExt.FindFile(findAuthor.GetFilePath()+"\\*.i7x");
      while (foundExt)
      {
        foundExt = findExt.FindNextFile();
        if (!findExt.IsDirectory())
          extensions.insert(Location(findAuthor.GetFileName(),findExt.GetFileTitle(),findExt.GetFilePath()));
      }

      // Find directory extensions
      foundExt = findExt.FindFile(findAuthor.GetFilePath()+"\\*.i7xd");
      while (foundExt)
      {
        foundExt = findExt.FindNextFile();
        if (findExt.IsDirectory())
        {
          CFileFind findSource;
          if (findSource.FindFile(findExt.GetFilePath()+"\\Source\\*.i7x"))
          {
            findSource.FindNextFile();
            if (!findSource.IsDirectory())
              extensions.insert(Location(findAuthor.GetFileName(),findSource.GetFileTitle(),findSource.GetFilePath()));
          }
        }
      }
    }
  }
}

bool Extension::UnpackZip(const CString& zipPath, const CString& destPath, CString& extPath)
{
  // Open the zip file
  unzFile zipFile = unzOpen(zipPath);
  if (zipFile == NULL)
    return false;

  // Start with the first file in the zip file
  if (unzGoToFirstFile(zipFile) != UNZ_OK)
  {
    unzClose(zipFile);
    return false;
  }

  // Iterate over the files in the zip file
  while (true)
  {
    // Get the filename and size
    unz_file_info info;
    CString name;
    int result = unzGetCurrentFileInfo(zipFile,&info,
      name.GetBufferSetLength(_MAX_PATH),_MAX_PATH,NULL,0,NULL,0);
    name.ReleaseBuffer();
    if (result != UNZ_OK)
    {
      unzClose(zipFile);
      return false;
    }

    if (!name.IsEmpty())
    {
      // Skip extraneous MacOS files
      bool extract = true;
      if (TextFormat::StartsWith(name,"__MACOSX/"))
        extract = false;
      if (TextFormat::EndsWith(name,"/.DS_Store"))
        extract = false;

      // Extract from the zip file
      if (extract)
      {
        CString extractPath(destPath);
        extractPath.AppendFormat("\\%s",(LPCSTR)name);

        char end = name.GetAt(name.GetLength()-1);
        if ((end == '/') || (end == '\\'))
          ::SHCreateDirectoryEx(0,extractPath,NULL);
        else
        {
          if (unzOpenCurrentFile(zipFile) != UNZ_OK)
          {
            unzClose(zipFile);
            return false;
          }

          int length = info.uncompressed_size;
          void* buffer = alloca(length);
          if (unzReadCurrentFile(zipFile,buffer,length) != length)
          {
            unzClose(zipFile);
            return false;
          }

          if (unzCloseCurrentFile(zipFile) != UNZ_OK)
          {
            unzClose(zipFile);
            return false;
          }

          FILE* extractFile = fopen(extractPath,"wb");
          if (extractFile != NULL)
          {
            fwrite(buffer,1,length,extractFile);
            fclose(extractFile);
          }
          else
          {
            unzClose(zipFile);
            return false;
          }

          // If we've extracted a .i7x file, use it to work out the extension path
          if (TextFormat::EndsWith(name,".i7x"))
          {
            extPath.Format("%s\\%s",(LPCSTR)destPath,(LPCSTR)name);

            int sep = name.FindOneOf("/\\");
            if (sep > 0)
            {
              CString dir = name.Left(sep);
              if (TextFormat::EndsWith(dir,".i7xd"))
                extPath.Format("%s\\%s",(LPCSTR)destPath,(LPCSTR)dir);
            }
          }
        }
      }
    }

    // Move to the next file in the zip file
    switch (unzGoToNextFile(zipFile))
    {
    case UNZ_OK:
      break;
    case UNZ_END_OF_LIST_OF_FILE:
      unzClose(zipFile);
      return !(extPath.IsEmpty());
    default:
      unzClose(zipFile);
      return false;
    }
  }

  return false;
}

bool Extension::IsBuiltIn(const char* path)
{
  // Check if the extension is under the program directory
  CString appDir = theApp.GetAppDir();
  return (strncmp(path,appDir,appDir.GetLength()) == 0);
}

bool Extension::IsLegacy(const char* path)
{
  // Check if the extension is under the legacy extensions directory
  CString legacyDir;
  legacyDir.Format("%s\\Inform\\Extensions",(LPCSTR)theApp.GetHomeDir());
  return (strncmp(path,legacyDir,legacyDir.GetLength()) == 0);
}

const Extension::Array& Extension::Legacy::Get(void)
{
  return theApp.GetExtensionCensus().GetExtensions();
}

void Extension::Legacy::Install(CFrameWnd* parent)
{
  // Ask the user for an extension
  SimpleFileDialog dialog(TRUE,"i7x",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,
    "Inform extensions (*.i7x)|*.i7x|All Files (*.*)|*.*||",parent);
  dialog.m_ofn.lpstrTitle = "Select the extension to install";
  if (dialog.DoModal() != IDOK)
    return;

  // Check for a valid extension
  CString path = dialog.GetPathName();
  CStringW extName, extAuthor, extVersion;
  if (!ReadFirstLine(path,extName,extAuthor,extVersion))
  {
    CString msg;
    msg.Format(
      "The file \"%s\"\n"
      "does not seem to be an extension. Extensions should be\n"
      "saved as UTF-8 format text files, and should start with a\n"
      "line of one of these forms:\n\n"
      "<Extension> by <Author> begins here.\n"
      "Version <Version> of <Extension> by <Author> begins here.",
      (LPCSTR)path);
    parent->MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Work out the path to copy the extension to
  CString target;
  target.Format("%s\\Inform\\Extensions\\%S",(LPCSTR)theApp.GetHomeDir(),(LPCWSTR)extAuthor);
  ::CreateDirectory(target,NULL);
  target.AppendFormat("\\%S.i7x",(LPCWSTR)extName);

  // Check if the extension already exists
  if (::GetFileAttributes(target) != INVALID_FILE_ATTRIBUTES)
  {
    CString msg;
    msg.Format(
      "A version of the extension %S by %S is already installed.\n"
      "Do you want to overwrite the installed extension with this new one?",
      (LPCWSTR)extName,(LPCWSTR)extAuthor);
    if (parent->MessageBox(msg,INFORM_TITLE,MB_ICONWARNING|MB_YESNO) != IDYES)
      return;
  }

  // Copy the extension
  if (::CopyFile(path,target,FALSE) == 0)
  {
    parent->MessageBox("Failed to copy extension",INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Update the extensions menu
  theApp.GetExtensionCensus().Run();

  // Tell the user
  CString msg;
  msg.Format("\"%S\" by %S has been installed",(LPCWSTR)extName,(LPCWSTR)extAuthor);
  parent->MessageBox(msg,INFORM_TITLE,MB_ICONINFORMATION|MB_OK);
}

bool Extension::Legacy::ReadFirstLine(const char* path, CStringW& name, CStringW& author, CStringW& version)
{
  // Get the first line of the file
  CStdioFile extFile;
  if (!extFile.Open(path,CFile::modeRead|CFile::typeBinary))
    return false;
  CString firstLineUTF8;
  if (!extFile.ReadString(firstLineUTF8))
    return false;
  extFile.Close();
  firstLineUTF8.Trim();

  // Check for a line-end
  int lePos = firstLineUTF8.FindOneOf("\r\n");
  if (lePos != -1)
    firstLineUTF8.Truncate(lePos);

  // Check for a Unicode line-end
  lePos = firstLineUTF8.Find("\xE2\x80\xA8");
  if (lePos != -1)
    firstLineUTF8.Truncate(lePos);

  // Check for a UTF-8 BOM
  if (firstLineUTF8.GetLength() >= 3)
  {
    if (firstLineUTF8.Left(3) == "\xEF\xBB\xBF")
      firstLineUTF8 = firstLineUTF8.Mid(3);
  }

  // Convert from UTF-8 to Unicode
  CStringW firstLine = TextFormat::UTF8ToUnicode(firstLineUTF8);

  // Split the first line into tokens
  CArray<CStringW> tokens;
  int pos = 0;
  CStringW token = firstLine.Tokenize(L" \t",pos);
  while (token.IsEmpty() == FALSE)
  {
    tokens.Add(token);
    token = firstLine.Tokenize(L" \t",pos);
  }

  // Remove leading "Version XYZ of", if present
  if (tokens.GetSize() == 0)
    return false;
  if (tokens[0] == L"Version")
  {
    if (tokens.GetSize() < 3)
      return false;
    if (tokens[2] != L"of")
      return false;
    version.Format(L"%s %s",tokens[0],tokens[1]);
    tokens.RemoveAt(0);
    tokens.RemoveAt(0);
    tokens.RemoveAt(0);
  }
  else
    version = L"Version (none)";

  // Remove trailing "begins here", if present
  int size = (int)tokens.GetSize();
  if (size < 2)
    return false;
  if (tokens[size-1] != L"here.")
    return false;
  if ((tokens[size-2] != L"begin") && (tokens[size-2] != L"begins"))
    return false;
  tokens.SetSize(size-2);

  // Remove leading "the" from the name, if present
  if (tokens.GetSize() == 0)
    return false;
  if ((tokens[0] == L"The") || (tokens[0] == L"the"))
    tokens.RemoveAt(0);

  // Extract the name and author
  bool gotName = false;
  bool gotBracket = false;
  while (tokens.GetSize() > 0)
  {
    if (tokens[0] == L"by")
    {
      gotName = true;
    }
    else if (gotName)
    {
      if (author.GetLength() > 0)
        author.AppendChar(L' ');
      author.Append(tokens[0]);
    }
    else if (!gotBracket)
    {
      if (tokens[0].GetAt(0) == '(')
        gotBracket = true;
      else
      {
        if (name.GetLength() > 0)
          name.AppendChar(L' ');
        name.Append(tokens[0]);
      }
    }
    tokens.RemoveAt(0);
  }

  if (name.IsEmpty() || author.IsEmpty() || version.IsEmpty())
    return false;
  return true;
}

void Extension::Legacy::Census::Run(void)
{
  // Don't start a new census if one is already running
  if (m_proc.process != INVALID_HANDLE_VALUE)
    return;

  // Create a pipe to read inbuild's output
  SECURITY_ATTRIBUTES security;
  ::ZeroMemory(&security,sizeof security);
  security.nLength = sizeof security;
  security.bInheritHandle = TRUE;
  ::CreatePipe(&m_read,&m_write,&security,0);

  CString appDir = theApp.GetAppDir();
  CString homeDir = theApp.GetHomeDir();
  CString command;
  command.Format("\"%s\\Compilers\\inbuild\" -inspect -recursive -contents-of \"%s\\Inform\\Extensions\" -internal \"%s\\Internal\" -json -",
    (LPCSTR)appDir,(LPCSTR)homeDir,(LPCSTR)appDir);

  STARTUPINFO start;
  ::ZeroMemory(&start,sizeof start);
  start.cb = sizeof start;
  start.hStdOutput = m_write;
  start.hStdError = m_write;
  start.wShowWindow = SW_HIDE;
  start.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  m_proc = theApp.CreateProcess(NULL,command,start,true);
  m_output.Empty();
}

void Extension::Legacy::Census::HandleEvent(void)
{
  if (m_proc.process != INVALID_HANDLE_VALUE)
  {
    // Read any census output
    CString output = theApp.ReadFromPipe(m_read);
    if (!output.IsEmpty())
      m_output.Append(output);

    // Is the census still running?
    DWORD result = STILL_ACTIVE;
    ::GetExitCodeProcess(m_proc.process,&result);
    if (result != STILL_ACTIVE)
    {
      // Wait for the process to end and read any final output
      theApp.WaitForProcessEnd(m_proc.process);
      theApp.GetTraceForProcess(m_proc.processId);
      output = theApp.ReadFromPipe(m_read);
      if (!output.IsEmpty())
        m_output.Append(output);

      // Close all handles associated with the census process
      m_proc.close();
      ::CloseHandle(m_read);
      m_read = INVALID_HANDLE_VALUE;
      ::CloseHandle(m_write);
      m_write = INVALID_HANDLE_VALUE;

      // If the census was successful, use the output
      if (result == 0)
      {
        m_extensions.clear();

        // Parse the JSON output from inbuild
        try
        {
          auto json = nlohmann::json::parse((LPCSTR)m_output);
          for (auto& inspect : json["inspection"])
          {
            auto& resource = inspect["resource"];
            if ((resource["type"] == "extension") && inspect.contains("location-file"))
            {
              std::string title = resource["title"];
              std::string author = resource["author"];
              std::string location = inspect["location-file"];
              if (!title.empty() && !author.empty() && !location.empty())
                m_extensions.push_back(Extension::Location(author.c_str(),title.c_str(),location.c_str()));
            }
          }
        }
        catch (std::exception& ex)
        {
          TRACE("Error parsing inbuild JSON output: %s\n",ex.what());
        }

        std::sort(m_extensions.begin(),m_extensions.end());
        theApp.SendAllFrames(InformApp::Extensions,0);
      }
      m_output.Empty();
    }
  }
}

const Extension::Array& Extension::Legacy::Census::GetExtensions(void)
{
  return m_extensions;
}
