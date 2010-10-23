void FatalError(void);

void __cdecl _purecall(void)
{
  FatalError();
}
