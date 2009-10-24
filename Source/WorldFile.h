#pragma once

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////


class WorldFile
{
public:
    WorldFile();
    ~WorldFile();

    enum ECell
    {
        INVALID_CELL = -1, 
        EMPTY_CELL = 0, 
        OCCUPIED_CELL = 1, 
        CELL_MAX
    };

    bool        Load(const LPCWSTR szFilename);

    ECell       operator () ( int row, int col ) const;
	int         GetWidth()  { return m_cx; }
	int         GetHeight() { return m_cy; }

private:
    int m_cx, m_cy;
    ECell* m_pGrid;
};
