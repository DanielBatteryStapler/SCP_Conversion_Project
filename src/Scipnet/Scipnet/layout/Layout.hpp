#ifndef LAYOUT_H_INCLUDED
#define LAYOUT_H_INCLUDED

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <iostream>
#include <codecvt>
#include <iterator>
#include <algorithm>
#include "nlohmann/json.hpp"
#include <locale>

inline void failInvalidFunction(){
	throw std::runtime_error{"Function not implement because this function does not make sense in this context"};
}

inline std::string to_utf8(std::u32string const & s){
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
	return conv.to_bytes(s);
}

inline std::u32string to_utf32(std::string const & s){
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
	return conv.from_bytes(s);
}

struct GlyphAttributes{
	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool colorSet = false;
	struct Color{
		int red = 255;
		int green = 255;
		int blue = 255;
		bool operator==(const Color& a){
            return red == a.red && green == a.green && blue == a.blue;
		}
	};
	Color color;
	bool operator==(const GlyphAttributes& a){
        return bold == a.bold && italic == a.italic && underline == a.underline && colorSet == a.colorSet && color == a.color;
	}
};

struct OutputBuffer{
	int width;
	int height;

	struct Grid{
		std::string glyph;
		GlyphAttributes attributes;
	};

	std::vector<Grid> buffer;

	void setSize(int _width, int _height);
	void write(int x, int y, std::string c, GlyphAttributes attributes = {});
	void weakWrite(int x, int y, std::string c, GlyphAttributes attributes = {});
	Grid read(int x, int y);

	void print();
	nlohmann::json makeJsonOutput(int xPos, int yPos, int width, int height, int cursorXPos = -1, int cursorYPos = -1);
};

//actual Layout stuff

struct Rect{
	int x;
	int y;
	int width;
	int height;
};

struct Element{
	enum class Type{Inline, Block, Float};
	enum class Align{Left, Right, Center};
	enum class Direction{Left, Right};

	int widthLimit;
	Rect bounds;

    virtual ~Element() = default;

	virtual Type getType() = 0;
	virtual Direction getFloatDirection() = 0;
	virtual void addElement(std::shared_ptr<Element> newElement) = 0;
	virtual std::vector<std::shared_ptr<Element>> getChildList() = 0;
	virtual	bool isClickable() = 0;
	virtual nlohmann::json click() = 0;

	virtual void setChildrenWidthLimit() = 0;
	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds = {}) = 0;
	virtual void calculateBranchesPositionalGeometry() = 0;
	virtual void draw(OutputBuffer& buffer) = 0;
};

void calculateBoundingGeometryAsBlockParent(int& height, int& width, int widthLimit, std::vector<Rect> parentFloatBounds, std::vector<std::shared_ptr<Element>>& branches);
void calculateBranchesPositionalGeometryAsBlockParent(int x, int y, std::vector<std::shared_ptr<Element>>& branches);

static void adjustParentFloatsBounds(std::vector<Rect>& parentFloatBounds, int x, int y){
	for(Rect& rect : parentFloatBounds){
		rect.x -= x;
		rect.y -= y;
	}
}

struct Text : public Element{
	std::string text;
	GlyphAttributes attributes;
	std::string link;

	virtual Type getType(){ return Type::Inline; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return {}; }

	virtual	bool isClickable(){
		return link != "";
	}

	virtual nlohmann::json click(){
		if(link == ""){
			failInvalidFunction();
			return {};
		}
		else{
			nlohmann::json output;
			output["type"] = "link";
			output["url"] = link;
			return output;
		}
	}

	virtual void setChildrenWidthLimit(){
		//do nothing
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		bounds.height = 1;
		std::u32string utf32String = to_utf32(text);
		//I am aware that one utf32 character does not necessarily equal one glyph
		//but it's gonna work just fine for now
		bounds.width = utf32String.size();
	}

	virtual void calculateBranchesPositionalGeometry(){
		//do nothing
	}

	virtual void draw(OutputBuffer& buffer){
		std::u32string utf32String = to_utf32(text);
		for(int i = 0; i < utf32String.size(); i++){
			buffer.write(bounds.x + i, bounds.y, to_utf8(std::u32string{utf32String[i]}), attributes);
		}
	}
};

static std::shared_ptr<Text> makeText(std::string text, GlyphAttributes attributes = {}, std::string link = ""){
	{
		std::string temp = text;
		text = "";
		for(char c : temp){
			if(c == '\t'){
				text += ' ';
			}
			else{
				text += c;
			}
		}
	}

	std::shared_ptr<Text> output = std::make_unique<Text>();
	output->text = text;
	output->attributes = attributes;
	output->link = link;
	return output;
}

struct Newline : public Element{

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return {}; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){
		//do nothing
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		bounds.height = 0;
		bounds.width = 0;
	}

	virtual void calculateBranchesPositionalGeometry(){
		//do nothing
	}

	virtual void draw(OutputBuffer& buffer){
		//do nothing
	}
};

static std::shared_ptr<Newline> makeNewline(){
	std::shared_ptr<Newline> output = std::make_unique<Newline>();
	return output;
}

struct DivBlock : public Element{
	Align align;

	int topMargin;
	int rightMargin;
	int bottomMargin;
	int leftMargin;

	std::vector<std::shared_ptr<Element>> branches;

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){
		branches.push_back(newElement);
	}

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return branches; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){
		for(auto& element : branches){
			element->widthLimit = widthLimit - leftMargin - rightMargin;
			element->setChildrenWidthLimit();
		}
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		adjustParentFloatsBounds(parentFloatBounds, leftMargin, topMargin);
		bounds.width = widthLimit;
		int dontCareWidth;
		calculateBoundingGeometryAsBlockParent(bounds.height, dontCareWidth, widthLimit - leftMargin - rightMargin, parentFloatBounds, branches);
		bounds.height += topMargin + bottomMargin;
	}

	virtual void calculateBranchesPositionalGeometry(){
		calculateBranchesPositionalGeometryAsBlockParent(bounds.x + leftMargin, bounds.y + topMargin, branches);
	}

	virtual void draw(OutputBuffer& buffer){
		for(auto& element : branches){
			element->draw(buffer);
		}
	}
};

static std::shared_ptr<DivBlock> makeDivBlock(Element::Align align, std::vector<std::shared_ptr<Element>> branches, int topMargin, int rightMargin, int bottomMargin, int leftMargin){
	std::shared_ptr<DivBlock> output = std::make_unique<DivBlock>();
	output->align = align;
	output->branches = std::move(branches);

	output->topMargin = topMargin;
	output->rightMargin = rightMargin;
	output->bottomMargin = bottomMargin;
	output->leftMargin = leftMargin;

	return output;
}


struct LimitedBlock : public Element{
	Align align;

	int maxWidth;
	bool fillExtra;

	std::vector<std::shared_ptr<Element>> branches;

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){
		branches.push_back(newElement);
	}

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return branches; }

	virtual void setChildrenWidthLimit(){
		for(auto& element : branches){
			if(widthLimit > maxWidth){
				element->widthLimit = maxWidth;
			}
			else{
				element->widthLimit = widthLimit;
			}
			element->setChildrenWidthLimit();
		}
	}

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		if(fillExtra){
			bounds.width = widthLimit;
		}
		else{
			bounds.width = std::min(widthLimit, maxWidth);
		}
		int dontCareWidth;
		if(widthLimit > maxWidth){
			if(fillExtra){
				adjustParentFloatsBounds(parentFloatBounds, ((widthLimit - maxWidth) / 2), 0);
			}
			calculateBoundingGeometryAsBlockParent(bounds.height, dontCareWidth, maxWidth, parentFloatBounds, branches);
		}
		else{
			calculateBoundingGeometryAsBlockParent(bounds.height, dontCareWidth, widthLimit, parentFloatBounds, branches);
		}

	}

	virtual void calculateBranchesPositionalGeometry(){
		if(widthLimit > maxWidth && fillExtra == true){
			calculateBranchesPositionalGeometryAsBlockParent(bounds.x + ((widthLimit - maxWidth) / 2), bounds.y, branches);
		}
		else{
			calculateBranchesPositionalGeometryAsBlockParent(bounds.x, bounds.y, branches);
		}
	}

	virtual void draw(OutputBuffer& buffer){
		for(auto& element : branches){
			element->draw(buffer);
		}
	}
};

static std::shared_ptr<LimitedBlock> makeLimitedBlock(Element::Align align, std::vector<std::shared_ptr<Element>> branches, int maxWidth, bool fillExtra){
	std::shared_ptr<LimitedBlock> output = std::make_unique<LimitedBlock>();
	output->align = align;
	output->branches = std::move(branches);

	output->maxWidth = maxWidth;
	output->fillExtra = fillExtra;

	return output;
}

struct Float : public Element{
	std::vector<std::shared_ptr<Element>> branches;

	Direction direction;

	virtual Type getType(){ return Type::Float; }

	virtual Direction getFloatDirection(){ return direction; }

	virtual void addElement(std::shared_ptr<Element> newElement){
		branches.push_back(newElement);
	}

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return branches; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){
		for(auto& element : branches){
			element->widthLimit = widthLimit;
			element->setChildrenWidthLimit();
		}
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		calculateBoundingGeometryAsBlockParent(bounds.height, bounds.width, widthLimit, {}, branches);
	}

	virtual void calculateBranchesPositionalGeometry(){
		calculateBranchesPositionalGeometryAsBlockParent(bounds.x, bounds.y, branches);
	}

	virtual void draw(OutputBuffer& buffer){
		for(auto& element : branches){
			element->draw(buffer);
		}
	}
};

static std::shared_ptr<Float> makeFloat(Element::Direction direction, std::vector<std::shared_ptr<Element>> branches){
	std::shared_ptr<Float> output = std::make_unique<Float>();
	output->direction = direction;
	output->branches = std::move(branches);
	return output;
}

struct InlineBlock : public Element{
	std::vector<std::shared_ptr<Element>> branches;

	int topMargin;
	int rightMargin;
	int bottomMargin;
	int leftMargin;

	virtual Type getType(){ return Type::Inline; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){
		branches.push_back(newElement);
	}

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return branches; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){
		for(auto& element : branches){
			element->widthLimit = widthLimit - rightMargin - leftMargin;
			element->setChildrenWidthLimit();
		}
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		calculateBoundingGeometryAsBlockParent(bounds.height, bounds.width, widthLimit - rightMargin - leftMargin, {}, branches);
		bounds.height += topMargin + bottomMargin;
		bounds.width += leftMargin + rightMargin;
	}

	virtual void calculateBranchesPositionalGeometry(){
		calculateBranchesPositionalGeometryAsBlockParent(bounds.x + leftMargin, bounds.y + topMargin, branches);
	}

	virtual void draw(OutputBuffer& buffer){
		for(auto& element : branches){
			element->draw(buffer);
		}
	}
};

static std::shared_ptr<InlineBlock> makeInlineBlock(std::vector<std::shared_ptr<Element>> branches, int topMargin, int rightMargin, int bottomMargin, int leftMargin){
	std::shared_ptr<InlineBlock> output = std::make_unique<InlineBlock>();
	output->branches = std::move(branches);

	output->topMargin = topMargin;
	output->rightMargin = rightMargin;
	output->bottomMargin = bottomMargin;
	output->leftMargin = leftMargin;

	return output;
}

static std::shared_ptr<Element> makeClearLine(){
	return makeDivBlock(Element::Align::Left, {makeInlineBlock({makeDivBlock(Element::Align::Left, {}, 0, 0, 0, 0)}, 0, 0, 0, 0)}, 0, 0, 0, 0);
}

struct Table : public Element{
	struct Cell{
		int columnSpan;
		std::shared_ptr<Element> element;
	};

	std::vector<std::vector<Cell>> tableRows;
	bool drawLines;
	bool hasLines;

	virtual Type getType(){ return Type::Inline; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){
		std::vector<std::shared_ptr<Element>> children;
		for(auto i : tableRows){
			for(auto y : i){
				children.push_back(y.element);
			}
		}
		return children;
	}

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit();

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds);

	virtual void calculateBranchesPositionalGeometry();

	virtual void draw(OutputBuffer& buffer);
};

static std::shared_ptr<Table> makeTable(bool drawLines, bool hasLines){
	std::shared_ptr<Table> output = std::make_unique<Table>();

	output->drawLines = drawLines;
	output->hasLines = hasLines;
	if(drawLines == true && hasLines == false){
		throw std::runtime_error{"Table: cannot draw lines without having lines"};
	}

	return output;
}


struct ImageCaptionTable : public Element{
	
	std::shared_ptr<Element> imageElement;
	std::shared_ptr<Element> captionElement;

	virtual Type getType(){ return Type::Inline; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){
		return {imageElement, captionElement};
	}

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit();

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds);

	virtual void calculateBranchesPositionalGeometry();

	virtual void draw(OutputBuffer& buffer);
};

static std::shared_ptr<ImageCaptionTable> makeImageCaptionTable(std::shared_ptr<Element> image, std::shared_ptr<Element> caption){
	std::shared_ptr<ImageCaptionTable> output = std::make_unique<ImageCaptionTable>();
	
	output->imageElement = image;
	output->captionElement = caption;

	return output;
}

struct Image : public Element{
	
	static inline double pixelGlyphRatioVertical = 14;
	static inline double pixelGlyphRatioHorizontal = 6;
	
	static inline bool useColor = true;
	static inline bool fullColor = false;
	
	struct Pixel{
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};
	
	int imageWidth;
	int imageHeight;
	std::vector<Pixel> pixelData; 
	int maxWidth;
	std::vector<OutputBuffer::Grid> renderedImage;
	
	virtual Type getType(){ return Type::Inline; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return {}; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){

	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		bounds.width = std::min<int>(maxWidth, widthLimit);
		bounds.height = (bounds.width * pixelGlyphRatioHorizontal * (static_cast<double>(imageHeight) / imageWidth)) / pixelGlyphRatioVertical;
		
		double horizontalRatio = static_cast<double>(imageWidth) / bounds.width;
		double verticalRatio = static_cast<double>(imageHeight) / bounds.height;
		
		renderedImage.clear();
		
		for(int y = 0; y < bounds.height; y++){
			for(int x = 0; x < bounds.width; x++){
				int red = 0;
				int green = 0;
				int blue = 0;
				int samples = 0;
				for(int dx = x * horizontalRatio; dx < (x + 1) * horizontalRatio; dx++){
					for(int dy = y * verticalRatio; dy < (y + 1) * verticalRatio; dy++){
						red += pixelData[dy * imageWidth + dx].red;
						green += pixelData[dy * imageWidth + dx].green;
						blue += pixelData[dy * imageWidth + dx].blue;
						samples++;
					}
				}
				red /= samples;
				green /= samples;
				blue /= samples;
				
				const std::string grayScaleChars = {" .-:=+*%@#"};
				
				if(useColor){
					int grayScale = (red + green + blue) / 3;
					OutputBuffer::Grid grid;
					//grid.glyph = "#";
					grid.glyph = grayScaleChars.at(grayScale / 256.0 * grayScaleChars.size());
					grid.attributes.colorSet = true;
					if(fullColor){
						grid.attributes.color.red = red;
						grid.attributes.color.green = green;
						grid.attributes.color.blue = blue;
					}
					else{
						grid.attributes.color.red = red & 0b11100000;
						grid.attributes.color.green = green & 0b11100000;
						grid.attributes.color.blue = blue & 0b11100000;
					}
					renderedImage.push_back(grid);
				}
				else{
					int grayScale = (red + green + blue) / 3;
					OutputBuffer::Grid grid;
					grid.glyph = grayScaleChars.at(grayScale / 256.0 * grayScaleChars.size());
					renderedImage.push_back(grid);
				}
			}
		}
	}

	virtual void calculateBranchesPositionalGeometry(){
		
	}

	virtual void draw(OutputBuffer& buffer){
		for(int y = 0; y < bounds.height; y++){
			for(int x = 0; x < bounds.width; x++){
				buffer.write(bounds.x + x, bounds.y + y, renderedImage[y * bounds.width + x].glyph, renderedImage[y * bounds.width + x].attributes);
			}
		}
	}
};

static std::shared_ptr<Image> makeImage(int imageWidth, int imageHeight, std::vector<Image::Pixel> pixelData, int maxWidth){
	std::shared_ptr<Image> output = std::make_unique<Image>();

	output->imageWidth = imageWidth;
	output->imageHeight = imageHeight;
	output->pixelData = pixelData;
	output->maxWidth = maxWidth;
	
	return output;
}

struct QuoteBox : public Element{
	std::vector<std::shared_ptr<Element>> branches;

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){
		branches.push_back(newElement);
	}

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return branches; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){
		for(auto& element : branches){
			element->widthLimit = widthLimit - 2;
			element->setChildrenWidthLimit();
		}
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		adjustParentFloatsBounds(parentFloatBounds, 1, 1);
		bounds.width = widthLimit;
		int dontCareWidth;
		calculateBoundingGeometryAsBlockParent(bounds.height, dontCareWidth, widthLimit - 2, parentFloatBounds, branches);
		bounds.height += 2;
	}

	virtual void calculateBranchesPositionalGeometry(){
		calculateBranchesPositionalGeometryAsBlockParent(bounds.x + 1, bounds.y + 1, branches);
	}

	virtual void draw(OutputBuffer& buffer);
};

static std::shared_ptr<QuoteBox> makeQuoteBox(){
	std::shared_ptr<QuoteBox> output = std::make_unique<QuoteBox>();

	return output;
}

struct HorizontalLine : public Element{

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return {}; }

	virtual	bool isClickable(){ return false; };

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; };

	virtual void setChildrenWidthLimit(){
		//do nothing
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		bounds.width = widthLimit;
		bounds.height = 1;
	}

	virtual void calculateBranchesPositionalGeometry(){
		//do nothing
	}

	virtual void draw(OutputBuffer& buffer){
		for(int x = 0; x < bounds.width; x++){
			buffer.weakWrite(bounds.x + x , bounds.y, "-");
		}
	}
};

static std::shared_ptr<HorizontalLine> makeHorizontalLine(){
	std::shared_ptr<HorizontalLine> output = std::make_unique<HorizontalLine>();

	return output;
}

struct Button : public Element{
	std::vector<std::shared_ptr<Element>> branches;
	std::function<void(void)> clickFunction;

	virtual Type getType(){ return Type::Inline; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){
		branches.push_back(newElement);
	}

	virtual std::vector<std::shared_ptr<Element>> getChildList(){ return branches; }

	virtual	bool isClickable(){ return true; }

	virtual nlohmann::json click(){
		clickFunction();
		return nullptr;
	}

	virtual void setChildrenWidthLimit(){
		for(auto& element : branches){
			element->setChildrenWidthLimit();
		}
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		calculateBoundingGeometryAsBlockParent(bounds.height, bounds.width, widthLimit, {}, branches);
	}

	virtual void calculateBranchesPositionalGeometry(){
		calculateBranchesPositionalGeometryAsBlockParent(bounds.x, bounds.y, branches);
	}

	virtual void draw(OutputBuffer& buffer){
		for(auto& element : branches){
			element->draw(buffer);
		}
	}
};

static std::shared_ptr<Button> makeButton(std::vector<std::shared_ptr<Element>> branches, std::function<void(void)> clickFunction){
	std::shared_ptr<Button> output = std::make_unique<Button>();
	output->branches = std::move(branches);
	output->clickFunction = clickFunction;

	return output;
}

struct Collapsible : public Element{
	std::shared_ptr<Element> closedButton;
	std::shared_ptr<Element> openedButton;
	std::shared_ptr<Element> content;

	bool isOpen;

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){
		std::vector<std::shared_ptr<Element>> output;
		if(isOpen){
			output.push_back(openedButton);
			output.push_back(content);
		}
		else{
			output.push_back(closedButton);
		}
		return output;
	}

	virtual	bool isClickable(){ return false; }

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; }

	virtual void setChildrenWidthLimit(){
		closedButton->widthLimit = widthLimit;
		closedButton->setChildrenWidthLimit();

		openedButton->widthLimit = widthLimit;
		openedButton->setChildrenWidthLimit();

		content->widthLimit = widthLimit;
		content->setChildrenWidthLimit();
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
		bounds.width = widthLimit;
		if(isOpen){
			openedButton->calculateBoundingGeometry(parentFloatBounds);
			adjustParentFloatsBounds(parentFloatBounds, 0, openedButton->bounds.height);
			content->calculateBoundingGeometry(parentFloatBounds);
			bounds.height = openedButton->bounds.height + content->bounds.height;
		}
		else{
			closedButton->calculateBoundingGeometry(parentFloatBounds);
			bounds.height = closedButton->bounds.height;
		}
	}

	virtual void calculateBranchesPositionalGeometry(){
		if(isOpen){
			openedButton->bounds.x = bounds.x;
			openedButton->bounds.y = bounds.y;
			openedButton->calculateBranchesPositionalGeometry();

			content->bounds.x = bounds.x;
			content->bounds.y = bounds.y + openedButton->bounds.height;
			content->calculateBranchesPositionalGeometry();
		}
		else{
			closedButton->bounds.x = bounds.x;
			closedButton->bounds.y = bounds.y;
			closedButton->calculateBranchesPositionalGeometry();
		}
	}

	virtual void draw(OutputBuffer& buffer){
		if(isOpen){
			openedButton->draw(buffer);
			content->draw(buffer);
		}
		else{
			closedButton->draw(buffer);
		}
	}
};

static std::shared_ptr<Collapsible> makeCollapsible(std::shared_ptr<Element> closedElement, std::shared_ptr<Element> openedElement, std::shared_ptr<Element> content, bool isOpen){
	std::shared_ptr<Collapsible> output = std::make_unique<Collapsible>();
	Collapsible* collapsible = output.get();

	output->closedButton = makeButton({closedElement}, [collapsible](){collapsible->isOpen = true;});
	output->openedButton = makeButton({openedElement}, [collapsible](){collapsible->isOpen = false;});
	output->content = content;
	output->isOpen = isOpen;

	return output;
}

struct TabView : public Element{
	struct Tab{
		std::shared_ptr<Text> text;
		std::shared_ptr<Element> content;
		std::shared_ptr<Button> button;
	};

	std::vector<Tab> tabs;
	int currentTab = 0;

	virtual Type getType(){ return Type::Block; }

	virtual Direction getFloatDirection(){ failInvalidFunction(); return {}; }

	virtual void addElement(std::shared_ptr<Element> newElement){ failInvalidFunction(); }

	virtual std::vector<std::shared_ptr<Element>> getChildList(){
		std::vector<std::shared_ptr<Element>> output;
		for(int i = 0; i < tabs.size(); i++){
			if(i != currentTab){
				output.push_back(tabs[i].button);
			}
		}
		output.push_back(tabs[currentTab].content);
		return output;
	}

	virtual	bool isClickable(){ return false; }

	virtual nlohmann::json click(){ failInvalidFunction(); return {}; }

	virtual void setChildrenWidthLimit(){
		for(Tab& tab : tabs){
			tab.button->widthLimit = (widthLimit - (tabs.size() * 3) - 1) / tabs.size();
			tab.button->setChildrenWidthLimit();

			tab.content->widthLimit = widthLimit - 2;
			tab.content->setChildrenWidthLimit();
		}
	}

	virtual void calculateBoundingGeometry(std::vector<Rect> parentFloatBounds);

	virtual void calculateBranchesPositionalGeometry();

	virtual void draw(OutputBuffer& buffer);
};

static std::shared_ptr<TabView> makeTabView(std::vector<TabView::Tab> tabs){
	std::shared_ptr<TabView> output = std::make_unique<TabView>();
	TabView* tabView = output.get();

	int i = 0;
	for(TabView::Tab& tab : tabs){
		tab.button = makeButton({tab.text}, [i, tabView](){ tabView->currentTab = i; });
		i++;
	}

	output->tabs = tabs;

	return output;
}

/// util functions

OutputBuffer drawLayout(std::shared_ptr<Element> element);

std::vector<std::shared_ptr<Element>> getClickableElements(std::shared_ptr<Element> element);


#endif // LAYOUT_H_INCLUDED
