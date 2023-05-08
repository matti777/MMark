#include "MMarkTextRenderer.h"
#include "CommonFunctions.h"

static const int MMarkFontHeight = 64;

MMarkTextRenderer::MMarkTextRenderer(GLuint program, GLuint mvpLoc,
                                     GLuint textureLoc, GLuint indexBuffer)
    : TextRenderer(program, mvpLoc, textureLoc, indexBuffer, MMarkFontHeight)
{
}

MMarkTextRenderer::~MMarkTextRenderer()
{
}

// Used to create the actual AlphabetCharInfo structure
struct ConstructAlphabetInfo {
    // Char in question: 'A' etc
    wchar_t m_char;

    // Pixel index (x) of left side of the char
    int m_left;

    // Pixel index (x) of right side of the char
    int m_right;
};

// The font used is "Canadian Penguin"

ConstructAlphabetInfo MMarkAlphabetInfo[] = {
    { 'A', 0, 54 },
    { 'B', 54, 105 },
    { 'C', 105, 152 },
    { 'D', 152, 197 },
    { 'E', 197, 235 },
    { 'F', 235, 279 },
    { 'G', 300, 350 },
    { 'H', 350, 390 },
    { 'I', 390, 436 },
    { 'J', 436, 480 },
    { 'K', 480, 525 },
    { 'L', 525, 563 },
    { 'M', 563, 613 },
    { 'N', 613, 660 },
    { 'O', 660, 710 },
    { 'P', 710, 758 },
    { 'Q', 758, 816 },
    { 'R', 816, 859 },
    { 'S', 859, 902 },
    { 'T', 902, 953 },
    { 'U', 976, 1022 },
    { 'V', 1022, 1074 },
    { 'W', 1074, 1145 },
    { 'X', 1145, 1190 },
    { 'Y', 1190, 1243 },
    { 'Z', 1243, 1289 },
    { '0', 1289, 1327 },
    { '1', 1327, 1357 },
    { '2', 1357, 1389 },
    { '3', 1389, 1427 },
    { '4', 1427, 1464 },
    { '5', 1464, 1501 },
    { '6', 1502, 1547 },
    { '7', 1547, 1579 },
    { '8', 1579, 1628 },
    { '9', 1628, 1660 },
    { ',', 1660, 1678 },
    { '.', 1678, 1697 },
    { '-', 1700, 1735 },
    { '+', 1735, 1766 },
    { '(', 1766, 1784 },
    { ')', 1784, 1805 },
    { '=', 1805, 1832 },
    { '/', 1832, 1864 },
    { '_', 1872, 1925 },
    { '~', 1925, 1987 },
    { ' ', 1987, 2012 }
};

// Width of the font texture atlas
static const float TextureWidth = 2048.0;

bool MMarkTextRenderer::Setup()
{
    if ( !TextRenderer::Setup() )
    {
        return false;
    }

    // Load the texture atlas
    if ( !Load2DTextureFromBundle("font.png", &m_fontTextureAtlas,
                                true, false) )
    {
        return false;
    }

    // Create the alphabet info
    m_numAlphabet = sizeof(MMarkAlphabetInfo) / sizeof(ConstructAlphabetInfo);
    m_alphabet = new AlphabetCharInfo[m_numAlphabet];

    for ( int i = 0; i < m_numAlphabet; i++ )
    {
        m_alphabet[i].m_char = MMarkAlphabetInfo[i].m_char;
        m_alphabet[i].m_width = MMarkAlphabetInfo[i].m_right -
                                MMarkAlphabetInfo[i].m_left;
        m_alphabet[i].m_height = 64;
        m_alphabet[i].m_vtop = 1.0;
        m_alphabet[i].m_vbottom = 0.0;
        m_alphabet[i].m_uleft = MMarkAlphabetInfo[i].m_left / TextureWidth;
        m_alphabet[i].m_uright = MMarkAlphabetInfo[i].m_right / TextureWidth;
    }

    LOG_DEBUG("MMarkTextRenderer::Setup(): set up %d chars", m_numAlphabet);

    return true;
}

