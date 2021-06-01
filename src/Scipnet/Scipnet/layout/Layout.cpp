#include "Layout.hpp"

/// OutputBuffer

void OutputBuffer::setSize(int _width, int _height){
	width = _width;
	height = _height;
	buffer.clear();
	for(int i = 0; i < width * height; i++){
		buffer.push_back({});
	}
}

void OutputBuffer::write(int x, int y, std::string c, GlyphAttributes attributes){
	if(x < 0 || x >= width || y < 0 || y >= height){
		//throw std::runtime_error("Out of bounds write");
		return;
	}
	buffer[x + y * width] = Grid{c, attributes};
}

void OutputBuffer::weakWrite(int x, int y, std::string c, GlyphAttributes attributes){
	if(x < 0 || x >= width || y < 0 || y >= height){
		//throw std::runtime_error("Out of bounds write");
		return;
	}
	if(buffer[x + y * width].glyph == ""){
		buffer[x + y * width] = Grid{c, attributes};
	}
}

OutputBuffer::Grid OutputBuffer::read(int x, int y){
	if(x < 0 || x >= width || y < 0 || y >= height){
		//throw std::runtime_error("Out of bounds read");
		return Grid{" "};
	}
	Grid temp = buffer[x + y * width];
	if(temp.glyph == ""){
		return Grid{" "};
	}
	else{
		return temp;
	}
}

void OutputBuffer::print(){
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			const Grid& grid = buffer[x + y * width];
			bool normal = true;
			
			if(grid.attributes.bold){
				std::cout << "\x1B[1m";
				normal = false;
			}
			
			if(grid.attributes.italic){
				std::cout << "\x1B[3m";
				normal = false;
			}
			
			if(grid.attributes.underline){
				std::cout << "\x1B[4m";
				normal = false;
			}
			
			if(grid.attributes.colorSet){
				std::cout << "\x1B[38;2;" << grid.attributes.color.red << ";" << grid.attributes.color.green << ";" << grid.attributes.color.blue << "m";
			}
			
			if(grid.glyph == ""){
				std::cout << " ";
			}
			else{
				std::cout << grid.glyph;
			}
			
			if(!normal){
				std::cout << "\x1B[0m";
			}
		}
		std::cout << "\n";
	}
}

nlohmann::json OutputBuffer::makeJsonOutput(int xPos, int yPos, int width, int height, int cursorXPos, int cursorYPos){
	nlohmann::json output;
	output["type"] = "fullscreenPrint";
	if(cursorXPos == -1){
		output["cursorX"] = -1;
		output["cursorY"] = -1;
	}
	else{
		cursorXPos -= xPos;
		cursorYPos -= yPos;
		if(cursorXPos < 0 || cursorYPos < 0 || cursorXPos >= width || cursorYPos >= height){
			output["cursorX"] = -1;
			output["cursorY"] = -1;
		}
		else{
			output["cursorX"] = cursorXPos;
			output["cursorY"] = cursorYPos;
		}
	}
	
	nlohmann::json outputGrid = nlohmann::json::array();
	
	GlyphAttributes currentAttributes; 
	std::string currentGlyphs;
	
	for(int y = yPos; y < yPos + height; y++){
		for(int x = xPos; x < xPos + width; x++){
			Grid grid = read(x, y);
			
			if(grid.attributes == currentAttributes){
				currentGlyphs += grid.glyph;
			}
			else{
				{
					nlohmann::json gridJ;
					
					gridJ["glyphs"] = currentGlyphs;
					gridJ["bold"] = currentAttributes.bold;
					gridJ["italic"] = currentAttributes.italic;
					gridJ["underline"] = currentAttributes.underline;
					gridJ["colorSet"] = currentAttributes.colorSet;
					gridJ["red"] = currentAttributes.color.red;
					gridJ["green"] = currentAttributes.color.green;
					gridJ["blue"] = currentAttributes.color.blue;
					
					outputGrid.push_back(gridJ);
				}
				
				currentAttributes = grid.attributes;
				currentGlyphs = grid.glyph;
			}
		}
	}
	
	{
		nlohmann::json gridJ;
		
		gridJ["glyphs"] = currentGlyphs;
		gridJ["bold"] = currentAttributes.bold;
		gridJ["italic"] = currentAttributes.italic;
		gridJ["underline"] = currentAttributes.underline;
		gridJ["colorSet"] = currentAttributes.colorSet;
		gridJ["red"] = currentAttributes.color.red;
		gridJ["green"] = currentAttributes.color.green;
		gridJ["blue"] = currentAttributes.color.blue;
		
		outputGrid.push_back(gridJ);
	}
	
	output["grid"] = outputGrid;
	return output;
}

/// Util stuff

void calculateBoundingGeometryAsBlockParent(int& height, int& width, int widthLimit, std::vector<Rect> parentFloatBounds, std::vector<std::shared_ptr<Element>>& branches){
	std::vector<std::shared_ptr<Element>> currentSegment;
	
	const auto elementsIntersect = [](Rect* a, Rect* b){
		if((b->x >= a->x && b->x < a->x + a->width) || (a->x >= b->x && a->x < b->x + b->width)){
			if((b->y >= a->y && b->y < a->y + a->height) || (a->y >= b->y && a->y < b->y + b->height)){
				return true;
			}
		}
		return false;
	};
	
	const auto canPlace = [&currentSegment, &elementsIntersect, &parentFloatBounds](int index){
		
		Element::Type type = currentSegment[index]->getType();
		switch(type){
		case Element::Type::Block:
			failInvalidFunction();
		case Element::Type::Inline:
			{
				for(int i = 0; i < parentFloatBounds.size(); i++){
					if(elementsIntersect(&parentFloatBounds[i], &currentSegment[index]->bounds)){
						return false;
					}
				}
				for(int i = 0; i < currentSegment.size(); i++){
					if(currentSegment[i]->getType() == Element::Type::Float){
						if(elementsIntersect(&currentSegment[i]->bounds, &currentSegment[index]->bounds)){
							return false;
						}
					}
				}
				for(int i = 0; i < index; i++){
					if(currentSegment[i]->getType() == Element::Type::Inline){
						if(elementsIntersect(&currentSegment[i]->bounds, &currentSegment[index]->bounds)){
							return false;
						}
					}
				}
				return true;
			}
		case Element::Type::Float:
			{
				for(int i = 0; i < parentFloatBounds.size(); i++){
					if(elementsIntersect(&parentFloatBounds[i], &currentSegment[index]->bounds)){
						return false;
					}
				}
				for(int i = 0; i < index; i++){
					if(currentSegment[i]->getType() == Element::Type::Float){
						if(elementsIntersect(&currentSegment[i]->bounds, &currentSegment[index]->bounds)){
							return false;
						}
					}
				}
				return true;
			}
		}
	};
	
	int segmentY = 0;
	
	const auto processSegment = [&currentSegment, &segmentY, widthLimit, &canPlace, &parentFloatBounds](){
		for(int i = 0; i < currentSegment.size(); i++){//handle floats
			if(currentSegment[i]->getType() == Element::Type::Float){
				
				currentSegment[i]->bounds.y = segmentY;
				
				switch(currentSegment[i]->getFloatDirection()){
				case Element::Direction::Left:
					currentSegment[i]->bounds.x = 0;
					break;
				case Element::Direction::Right:
					currentSegment[i]->bounds.x = widthLimit - currentSegment[i]->bounds.width;
					break;
				}
				
				while(canPlace(i) == false){
					currentSegment[i]->bounds.y++;
				}
				Rect rect;
				rect.x = currentSegment[i]->bounds.x;
				rect.y = currentSegment[i]->bounds.y;
				rect.width = currentSegment[i]->bounds.width;
				rect.height = currentSegment[i]->bounds.height;
				parentFloatBounds.push_back(rect);
			}
		}
		
		int nextX = 0;
		int nextY = segmentY;
		int nextLineY = nextY + 1;
		
		for(int i = 0; i < currentSegment.size(); i++){//handle inline
			if(currentSegment[i]->getType() == Element::Type::Inline){
				currentSegment[i]->bounds.x = nextX;
				currentSegment[i]->bounds.y = nextY;
				
				if(currentSegment[i]->bounds.width <= widthLimit){
					while(canPlace(i) == false || currentSegment[i]->bounds.x + currentSegment[i]->bounds.width > widthLimit){
						currentSegment[i]->bounds.x++;
						if(currentSegment[i]->bounds.x + currentSegment[i]->bounds.width > widthLimit){
							currentSegment[i]->bounds.x = 0;
							currentSegment[i]->bounds.y = nextLineY;
							nextLineY++;
						}
					}
				}
				
				nextLineY = std::max(currentSegment[i]->bounds.y + currentSegment[i]->bounds.height, nextLineY);
				
				nextY = currentSegment[i]->bounds.y;
				nextX = currentSegment[i]->bounds.x + currentSegment[i]->bounds.width;
				if(nextX >= widthLimit){
					nextX = 0;
					nextY = nextLineY;
					nextLineY++;
				}
			}
		}
		
		for(auto& element : currentSegment){
			switch(element->getType()){
			case Element::Type::Block:
			case Element::Type::Inline:
				segmentY = std::max(element->bounds.height + element->bounds.y, segmentY);
				break;
			case Element::Type::Float:
				//do nothing
				break;
			}
		}
	};
	
	for(auto& element : branches){
		if(element->getType() == Element::Type::Block){
			//we've found a block, so process everything that came before it in the "currentSegment"
			processSegment();
			currentSegment.clear();
			
			//now actually place the block
			element->bounds.x = 0;
			element->bounds.y = segmentY;
			std::vector<Rect> floatRects = parentFloatBounds;
			for(Rect& rect : floatRects){
				rect.x -= element->bounds.x;
				rect.y -= element->bounds.y;
			}
			element->calculateBoundingGeometry(floatRects);
			segmentY += element->bounds.height;
		}
		else{
			element->calculateBoundingGeometry();
			currentSegment.push_back(element);
		}
	}
	
	processSegment();
	currentSegment.clear();
	
	height = segmentY;
	width = 0;
	
	for(auto& element : branches){
		width = std::max(element->bounds.width + element->bounds.x, width);
	}
}

void calculateBranchesPositionalGeometryAsBlockParent(int x, int y, std::vector<std::shared_ptr<Element>>& branches){
	for(auto& element : branches){
		element->bounds.x += x;
		element->bounds.y += y;
		element->calculateBranchesPositionalGeometry();
	}
}

/// Table

namespace{
	int getTableColumnCount(const std::vector<std::vector<Table::Cell>>& tableRows){
		int columnCount = 0;
		for(auto& row : tableRows){
			int rowColumnCount = 0;
			for(auto& cell : row){
				rowColumnCount += cell.columnSpan;
			}
			columnCount = std::max(columnCount, rowColumnCount);
		}
		return columnCount;
	}
	
	std::vector<int> getTableColumnWidths(const std::vector<std::vector<Table::Cell>>& tableRows, bool hasLines){
		std::vector<int> widths;
		widths.resize(getTableColumnCount(tableRows));
		
		for(auto& row : tableRows){
			int columnIndex = 0;
			for(auto& cell : row){
				int width = cell.element->bounds.width;
				
				for(int i = 1; i < cell.columnSpan; i++){
					width -= widths[columnIndex] + (hasLines?1:0);
					columnIndex++;
				}
				
				widths[columnIndex] = std::max(widths[columnIndex], width);
				
				columnIndex++;
			}
		}
		
		return widths;
	}
	
	int getTableWidth(const std::vector<std::vector<Table::Cell>>& tableRows, bool hasLines){
		std::vector<int> widths = getTableColumnWidths(tableRows, hasLines);
		int width = 0;
		for(int i : widths){
			width += i;
		}
		if(hasLines){
			width += widths.size() + 1;
		}
		return width;
	}
	
	std::vector<int> getTableRowHeights(const std::vector<std::vector<Table::Cell>>& tableRows){
		std::vector<int> heights;
		for(auto& row : tableRows){
			int height = 0;
			for(auto& cell : row){
				height = std::max(height, cell.element->bounds.height);
			}
			heights.push_back(height);
		}
		return heights;
	}
	
	int getTableHeight(const std::vector<std::vector<Table::Cell>>& tableRows, bool hasLines){
		std::vector<int> heights = getTableRowHeights(tableRows);
		int height = 0;
		for(int i : heights){
			height += i;
		}
		if(hasLines){
			height += heights.size() + 1;
		}
		return height;
	}
}

void Table::setChildrenWidthLimit(){
	int columnCount = getTableColumnCount(tableRows);
	for(auto& row : tableRows){
		for(auto& cell : row){
			cell.element->widthLimit = (widthLimit - columnCount - 1) / columnCount * cell.columnSpan;
			cell.element->setChildrenWidthLimit();
		}
	}
}

void Table::calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
	for(auto& row : tableRows){
		for(auto& cell : row){
			cell.element->calculateBoundingGeometry();
		}
	}
	
	//shift any extra space to the right
	{
		int columnCount = getTableColumnCount(tableRows);
		int normalColumnWidth = (widthLimit - columnCount - 1) / columnCount;
		std::vector<int> widths = getTableColumnWidths(tableRows, hasLines);
		int usedWidth = 0;
		for(int x = 0; x < columnCount - 1; x++){
			usedWidth += widths[x];
			int extra = (normalColumnWidth * (x + 1)) - usedWidth;
			
			if(extra > 0){
				for(auto& row : tableRows){
					int currentX = 0;
					for(auto& cell : row){
						currentX += cell.columnSpan;
						if(currentX > (x + 1)){
							cell.element->widthLimit += extra;
							cell.element->setChildrenWidthLimit();
							cell.element->calculateBoundingGeometry();
							break;
						}
					}
				}
				widths = getTableColumnWidths(tableRows, hasLines);
			}
		}
	}
	
	std::vector<int> heights = getTableRowHeights(tableRows);
	std::vector<int> yPoses;
	{
		int runningTotal = 0;
		for(int& height : heights){
			yPoses.push_back(runningTotal);
			runningTotal += height;
		}
	}
	
	std::vector<int> widths = getTableColumnWidths(tableRows, hasLines);
	std::vector<int> xPoses;
	{
		int runningTotal = 0;
		for(int& width : widths){
			xPoses.push_back(runningTotal);
			runningTotal += width;
		}
	}
	int y = 0;
	for(auto& row : tableRows){
		int x = 0;
		for(auto& cell : row){
			if(hasLines){
				cell.element->bounds.x = xPoses[x] + 1 + x;
				cell.element->bounds.y = yPoses[y] + 1 + y;
			}
			else{
				cell.element->bounds.x = xPoses[x];
				cell.element->bounds.y = yPoses[y];
			}
			x += cell.columnSpan;
		}
		y++;
	}
	
	bounds.width = getTableWidth(tableRows, hasLines);
	bounds.height = getTableHeight(tableRows, hasLines);
}

void Table::calculateBranchesPositionalGeometry(){
	for(auto& row : tableRows){
		for(auto& cell : row){
			cell.element->bounds.x += bounds.x;
			cell.element->bounds.y += bounds.y;
			cell.element->calculateBranchesPositionalGeometry();
		}
	}
}

void Table::draw(OutputBuffer& buffer){
	
	std::vector<int> widths = getTableColumnWidths(tableRows, hasLines);
	widths.push_back(0);
	std::vector<int> heights = getTableRowHeights(tableRows);
	heights.push_back(0);
	
	if(drawLines){
		buffer.write(bounds.x, bounds.y, "+");
		
		int yPos = bounds.y;
		for(int height : heights){
			int xPos = bounds.x;
			
			for(int width : widths){
				buffer.write(xPos, yPos, "+");
				for(int i = 0; i < width; i++){
					buffer.write(xPos + i + 1, yPos, "-");
				}
				xPos += width + 1;
			}
			
			for(int i = 0; i < height; i++){
				buffer.write(xPos - 1, yPos + i + 1, "|");
			}
			
			yPos += height + 1;
		}
	}
	
	int y = 0;
	for(auto& row : tableRows){
		int x = 0;
		for(auto& cell : row){
			if(drawLines){
				for(int i = 0; i < heights[y]; i++){
					buffer.write(cell.element->bounds.x - 1, cell.element->bounds.y + i, "|");
				}
			}
			
			cell.element->draw(buffer);
			x += cell.columnSpan;
		}
		y++;
	}
}

/// ImageCaptionTable

void ImageCaptionTable::setChildrenWidthLimit(){
	
	imageElement->widthLimit = widthLimit - 2;
	imageElement->setChildrenWidthLimit();
	captionElement->widthLimit = widthLimit - 2;//will get overrided later so that the caption fits the size of the image
	captionElement->setChildrenWidthLimit();
}

void ImageCaptionTable::calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
	imageElement->calculateBoundingGeometry();
	captionElement->widthLimit = imageElement->bounds.width;
	captionElement->setChildrenWidthLimit();
	captionElement->calculateBoundingGeometry();
	
	imageElement->bounds.x = 1;
	imageElement->bounds.y = 1;
	captionElement->bounds.x = 1;
	captionElement->bounds.y = 2 + imageElement->bounds.height;
	
	bounds.width = 2 + imageElement->bounds.width;
	bounds.height = 3 + imageElement->bounds.height + captionElement->bounds.height;
}

void ImageCaptionTable::calculateBranchesPositionalGeometry(){
	imageElement->bounds.x += bounds.x;
	imageElement->bounds.y += bounds.y;
	imageElement->calculateBranchesPositionalGeometry();
	
	captionElement->bounds.y += bounds.y;
	captionElement->bounds.x += bounds.x;
	captionElement->calculateBranchesPositionalGeometry();
}

void ImageCaptionTable::draw(OutputBuffer& buffer){
	for(int y = 1; y < bounds.height - 1; y++){
		for(int x = 1; x < bounds.width - 1; x++){
			buffer.write(bounds.x + x, bounds.y + y, " ");
		}
	}
	
	buffer.write(bounds.x, bounds.y, "+");
	buffer.write(bounds.x + bounds.width - 1, bounds.y, "+");
	buffer.write(bounds.x, bounds.y + bounds.height - 1, "+");
	buffer.write(bounds.x + bounds.width - 1, bounds.y + bounds.height - 1, "+");
	
	for(int y = 1; y < bounds.height - 1; y++){
		buffer.write(bounds.x, bounds.y + y, "|");
		buffer.write(bounds.x + bounds.width - 1, bounds.y + y, "|");
	}
	
	buffer.write(imageElement->bounds.width + imageElement->bounds.x, imageElement->bounds.height + imageElement->bounds.y, "+");
	buffer.write(bounds.x, imageElement->bounds.height + imageElement->bounds.y, "+");

	for(int x = 1; x < bounds.width - 1; x++){
		buffer.write(bounds.x + x, bounds.y, "-");
		buffer.write(bounds.x + x, imageElement->bounds.y + imageElement->bounds.height, "-");
		buffer.write(bounds.x + x, bounds.y + bounds.height - 1, "-");
	}
	
	imageElement->draw(buffer);
	captionElement->draw(buffer);
}

/// QuoteBox

void QuoteBox::draw(OutputBuffer& buffer){
	buffer.weakWrite(bounds.x, bounds.y, "+");
	buffer.weakWrite(bounds.x + bounds.width - 1, bounds.y, "+");
	buffer.weakWrite(bounds.x, bounds.y + bounds.height - 1, "+");
	buffer.weakWrite(bounds.x + bounds.width - 1, bounds.y + bounds.height - 1, "+");
	
	for(int x = 0; x < bounds.width - 2; x++){
		if(x % 2 == 0){
			buffer.weakWrite(bounds.x + x + 1, bounds.y, "-");
		}
		else{
			buffer.weakWrite(bounds.x + x + 1, bounds.y + bounds.height - 1, "-");
		}
	}
	
	for(int y = 0; y < bounds.height - 2; y++){
		if(y % 2 == 0){
			buffer.weakWrite(bounds.x, bounds.y + y + 1, "|");
		}
		else{
			buffer.weakWrite(bounds.x + bounds.width - 1, bounds.y + y + 1, "|");
		}
	}
	
	for(auto& element : branches){
		element->draw(buffer);
	}
}

/// TabView

void TabView::calculateBoundingGeometry(std::vector<Rect> parentFloatBounds){
	bounds.width = widthLimit;
	
	int currentX = 2;
	int tabHeight = 0;
	for(Tab& tab : tabs){
		tab.button->calculateBoundingGeometry({});
		tab.button->bounds.x = currentX;
		tab.button->bounds.y = 1;
		currentX += tab.button->bounds.width + 3;
		tabHeight = std::max(tabHeight, tab.button->bounds.height + 2);
	}
	
	tabs[currentTab].content->calculateBoundingGeometry({});
	tabs[currentTab].content->bounds.x = 1;
	tabs[currentTab].content->bounds.y = tabHeight;
	bounds.height = tabHeight + tabs[currentTab].content->bounds.height + 1;
}

void TabView::calculateBranchesPositionalGeometry(){
	for(Tab& tab : tabs){
		tab.button->bounds.x += bounds.x;
		tab.button->bounds.y += bounds.y;
		tab.button->calculateBranchesPositionalGeometry();
	}
	
	tabs[currentTab].content->bounds.x += bounds.x;
	tabs[currentTab].content->bounds.y += bounds.y;
	tabs[currentTab].content->calculateBranchesPositionalGeometry();
}

void TabView::draw(OutputBuffer& buffer){
	tabs[currentTab].content->draw(buffer);
	
	Rect contentBox = tabs[currentTab].content->bounds;
	contentBox.x--;
	contentBox.y--;
	contentBox.width += 2;
	contentBox.height += 2;
	
	buffer.write(contentBox.x, contentBox.y, "+");
	buffer.write(contentBox.x + contentBox.width - 1, contentBox.y, "+");
	buffer.write(contentBox.x, contentBox.y + contentBox.height - 1, "+");
	buffer.write(contentBox.x + contentBox.width - 1, contentBox.y + contentBox.height - 1, "+");
	
	for(int x = 0; x < contentBox.width - 2; x++){
		buffer.write(contentBox.x + x + 1, contentBox.y, "-");
		buffer.write(contentBox.x + x + 1, contentBox.y + contentBox.height - 1, "-");
	}
	
	for(int y = 0; y < contentBox.height - 2; y++){
		buffer.write(contentBox.x, contentBox.y + y + 1, "|");
		buffer.write(contentBox.x + contentBox.width - 1, contentBox.y + y + 1, "|");
	}
	
	for(int i = 0; i < tabs.size(); i++){
		Rect tabBox = tabs[i].button->bounds;
		tabBox.x--;
		tabBox.y--;
		tabBox.width += 2;
		tabBox.height += 2;
		
		buffer.write(tabBox.x, tabBox.y, "+");
		buffer.write(tabBox.x + tabBox.width - 1, tabBox.y, "+");
		
		for(int x = 0; x < tabBox.width - 2; x++){
				buffer.write(tabBox.x + x + 1, tabBox.y, "-");
		}
		
		for(int y = 0; y < tabBox.height - 2; y++){
			buffer.write(tabBox.x, tabBox.y + y + 1, "|");
			buffer.write(tabBox.x + tabBox.width - 1, tabBox.y + y + 1, "|");
		}
		
		if(i == currentTab){
			tabs[i].text->attributes.underline = false;
			buffer.write(tabBox.x, tabBox.y + tabBox.height - 1, "+");
			buffer.write(tabBox.x + tabBox.width - 1, tabBox.y + tabBox.height - 1, "+");
			for(int x = 0; x < tabBox.width - 2; x++){
				buffer.write(tabBox.x + x + 1, tabBox.y + tabBox.height - 1, " ");
			}
		}
		else{
			tabs[i].text->attributes.underline = true;
		}
		tabs[i].button->draw(buffer);
	}
}

/// util functions

OutputBuffer drawLayout(std::shared_ptr<Element> element){
	element->bounds.x = 0;
	element->bounds.y = 0;
	element->setChildrenWidthLimit();
	element->calculateBoundingGeometry();
	element->calculateBranchesPositionalGeometry();
	
	OutputBuffer buffer;
	buffer.setSize(element->bounds.width, element->bounds.height);
	element->draw(buffer);
	return buffer;
}

std::vector<std::shared_ptr<Element>> getClickableElements(std::shared_ptr<Element> element){
	std::vector<std::shared_ptr<Element>> output;
	
	if(element->isClickable()){
		output.push_back(element);
	}
	
	auto childList = element->getChildList();
	for(auto child : childList){
		auto newChildren = getClickableElements(child);
		output.insert(output.end(), newChildren.begin(), newChildren.end());
	}
	
	return output;
}



