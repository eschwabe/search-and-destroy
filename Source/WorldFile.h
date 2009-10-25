//------------------------------------------------------------------------------
// Project: Game Development (2009)
// 
// World File
//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------
// WorldFile Class
//------------------------------------------------------------------------------
class WorldFile
{
    public:
        
        // cell type
        enum ECell
        {
            INVALID_CELL = -1, 
            EMPTY_CELL = 0, 
            OCCUPIED_CELL = 1, 
            CELL_MAX
        };

        WorldFile();
        ~WorldFile();

        // load file
        bool Load(const LPCWSTR szFilename);

        // get cell type
        ECell operator () ( int row, int col ) const;

        // get grid size
	    int GetWidth() const { return m_cx; }
	    int GetHeight() const { return m_cy; }

    private:

        int m_cx, m_cy;
        ECell* m_pGrid;
};
