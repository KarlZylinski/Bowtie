#pragma once

namespace bowtie
{

class Font;
class Text
{
public:
	Text(const Font& font);

private:
	const Font& _font;

	Text(const Text&);
	Text& operator=(const Text&);
};

}