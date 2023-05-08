#include "ScoreTextRenderer.h"
#include "CommonFunctions.h"

// Width of the font texture atlas
static const float TextureWidth = 2048.0;
static const int FontHeight = 64;

ScoreTextRenderer::ScoreTextRenderer(GLuint program, GLuint mvpLoc,
                                     GLuint textureLoc, GLuint indexBuffer)
    : TextRenderer(program, mvpLoc, textureLoc, indexBuffer, FontHeight)
{
}

ScoreTextRenderer::~ScoreTextRenderer()
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

// Used font is "Neuropolitical"

ConstructAlphabetInfo AlphabetInfo[] = {
    { 'a', 2, 52 },
    { 'b', 52, 100 },
    { 'c', 100, 150 },
    { 'd', 150, 198 },
    { 'e', 198, 246 },
    { 'f', 246, 272 },
    { 'g', 272, 320 },
    { 'h', 320, 370 },
    { 'i', 370, 384 },
    { 'j', 394, 422 },
    { 'k', 422, 456 },
    { 'l', 456, 469 },
    { 'm', 469, 534 },
    { 'n', 534, 582 },
    { 'o', 582, 631 },
    { 'p', 631, 680 },
    { 'q', 680, 730 },
    { 'r', 730, 754 },
    { 's', 754, 798 },
    { 't', 798, 828 },
    { 'u', 828, 876 },
    { 'v', 877, 934 },
    { 'w', 934, 1002 },
    { 'x', 1002, 1044 },
    { 'y', 1044, 1091 },
    { 'z', 1092, 1137 },
    { '0', 1137, 1190 },
    { '1', 1190, 1202 },
    { '2', 1202, 1258 },
    { '3', 1258, 1314 },
    { '4', 1314, 1374 },
    { '5', 1374, 1430 },
    { '6', 1431, 1488 },
    { '7', 1489, 1542 },
    { '8', 1542, 1596 },
    { '9', 1596, 1650 },
    { ':', 1651, 1666 },
    { '.', 1666, 1680 },
    { ',', 1680, 1693 },
    { '+', 1693, 1720 },
    { '-', 1720, 1746 },
    { '=', 1746, 1772 },
    { '/', 1772, 1828 },
    { ' ', 1828, 1868 },
};

bool ScoreTextRenderer::Setup()
{
    if ( !TextRenderer::Setup() )
    {
        return false;
    }

    // Load the texture atlas
    if ( !Load2DTextureFromBundle("score_font.png", &m_fontTextureAtlas,
                                  true, false) )
    {
        return false;
    }

    // Create the alphabet info
    m_numAlphabet = sizeof(AlphabetInfo) / sizeof(ConstructAlphabetInfo);
    m_alphabet = new AlphabetCharInfo[m_numAlphabet];

    for ( int i = 0; i < m_numAlphabet; i++ )
    {
        m_alphabet[i].m_char = AlphabetInfo[i].m_char;
        m_alphabet[i].m_width = AlphabetInfo[i].m_right -
                                AlphabetInfo[i].m_left;
        m_alphabet[i].m_height = FontHeight;
        m_alphabet[i].m_vtop = 1.0;
        m_alphabet[i].m_vbottom = 0.0;
        m_alphabet[i].m_uleft = AlphabetInfo[i].m_left / TextureWidth;
        m_alphabet[i].m_uright = AlphabetInfo[i].m_right / TextureWidth;
    }

    LOG_DEBUG("ScoreTextRenderer::Setup(): set up %d chars", m_numAlphabet);

    return true;
}

