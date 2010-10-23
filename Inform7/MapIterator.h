#pragma once

template<typename KEY, typename ARG_KEY, typename VALUE, typename ARG_VALUE> class MapIterator
{
public:
  MapIterator(CMap<KEY,ARG_KEY,VALUE,ARG_VALUE>& map) : m_map(map)
  {
    m_pos = m_map.GetStartPosition();
  }

  bool Iterate(void)
  {
    if (m_pos == NULL)
      return false;
    m_map.GetNextAssoc(m_pos,m_key,m_value);
    return true;
  }

  bool Iterate(const CRuntimeClass* runClass)
  {
    while (true)
    {
      if (m_pos == NULL)
        return false;
      m_map.GetNextAssoc(m_pos,m_key,m_value);
      if (m_value->IsKindOf(runClass))
        return true;
    }
  }

  ARG_KEY Key(void)
  {
    return m_key;
  }

  ARG_VALUE Value(void)
  {
    return m_value;
  }

private:
  CMap<KEY,ARG_KEY,VALUE,ARG_VALUE>& m_map;
  POSITION m_pos;

  KEY m_key;
  VALUE m_value;
};
