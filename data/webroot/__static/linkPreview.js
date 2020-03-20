
window.onload = function(){
    var allLinks = document.getElementsByClassName("previewableLink");
    
    var anchorExists = function(element){
        var next = element.nextSibling;
        if(next !== null){
            if(typeof next.getAttribute !== "undefined" && next.getAttribute("class") == "linkPreviewAnchor"){
                return true;
            }
        }
        return false;
    }
    
    var initFunction = function(element){
        if(!anchorExists(element)){
            var source = element.getAttribute("href");
            if(source.length > 0 && source.charAt(0) == '/' && source.indexOf('.') == -1){
                source = source.substring(1, source.length);
                if(source.length > 0 && source.indexOf('/') == -1){
                    var previewAnchor = document.createElement('div');
                    previewAnchor.setAttribute("class", "linkPreviewAnchor");
                    var preview = document.createElement('div');
                    preview.setAttribute("class", "linkPreview");
                    var frame = document.createElement('iframe');
                    frame.setAttribute("class", "linkPreviewFrame");
                    frame.setAttribute("src", "about:blank");
                    frame.setAttribute("data-src", "/" + source + "/previewArticle");
                    frame.setAttribute("data-timer", "none");
                    frame.setAttribute("scrolling", "no");
                    preview.appendChild(frame);
                    previewAnchor.appendChild(preview);
                    element.parentNode.insertBefore(previewAnchor, element.nextSibling);
                }
            }
        }
    }
    
    var enterFunction = function(){
        if(anchorExists(this)){
            var frame = this.nextSibling.firstElementChild.firstElementChild;
            if(frame.getAttribute("data-src") != "loaded"){
                var timer = setTimeout(function(){
                    frame.setAttribute("src", frame.getAttribute("data-src"));
                    frame.setAttribute("data-src", "loaded");
                }, 1000);
                frame.setAttribute("data-timer", timer);
            }
        }
    };
    
    var leaveFunction = function(){
        if(anchorExists(this)){
            var frame = this.nextSibling.firstElementChild.firstElementChild;
            if(frame.getAttribute("data-src") != "loaded"){
                clearTimeout(+frame.getAttribute("data-timer"));
                frame.setAttribute("data-timer", "loaded");
            }
        }
    }

    for(var i = 0; i < allLinks.length; ++i){
        var element = allLinks[i];
        initFunction(element);
        element.addEventListener('mouseenter', enterFunction);
        element.addEventListener('mouseleave', leaveFunction);
    }
};
