#pragma once

namespace bowtie
{

struct Texture;
class Font
{
public:
	Font(const Texture& texture);

	const Texture& texture() const;

private:
	const Texture& _texture;

	Font(const Font&);
	Font& operator=(const Font&);
};

}