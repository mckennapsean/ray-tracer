// fsimage script for showing images fullscreen
// by Cem Yuksel (www.cemyuksel.com) 2013
// 
// Usage:
//  -- Do the following for each image you'd like add the fullscreen functionality:
//  <img rel="fsimage" src="[image_file]">
//  -- Also make sure to include this script:
//  <script type="text/javascript" src="http://www.cemyuksel.com/scripts/fsimage.js"></script>

var FSImage = {};

FSImage.Each = function(obj, callback) {var i = 0,len = obj.length;for (var value = obj[0]; i < len && callback.call(value, i, value) !== false; value = obj[++i]) {}}

FSImage.Init = function()
{
	var imgs = [];
	var id = 0;
	FSImage.Each(document.getElementsByTagName('img'), function (i,el) {
		var rel = el.getAttribute('rel');
		if (rel && rel=='fsimage') {
			imgs.push(el);
			var ix = id;
			el.onclick = function() { FSImage.Show(ix); }
			el.style.cursor = 'pointer';
			id++;
		}
	});
	FSImage.imgs = imgs;
	var f = document.createElement('div');
	f.style.position = 'fixed';
	f.style.left   = '0px';
	f.style.top    = '0px';
	f.style.width  = '100%';
	f.style.height = '100%';
	f.style.backgroundColor = 'black';
	f.style.zIndex = '100';
	f.style.display = 'none';
	f.onclick = function() { FSImage.Hide() }
	var i = document.createElement('div');
	i.style.width  = '100%';
	i.style.height = '100%';
	i.style.backgroundPosition = 'center';
	i.style.backgroundRepeat = 'no-repeat';
	i.style.backgroundSize = 'contain';
	i.style.cursor = 'crosshair';
	f.appendChild(i);
	FSImage.html = document.getElementsByTagName('html')[0];
	FSImage.body = document.getElementsByTagName('body')[0];
	FSImage.body.appendChild(f);
	FSImage.frame = f;
	FSImage.img = i;
	FSImage.imgID = -1;
}

FSImage.Show = function(id)
{
	FSImage.overflowHTML = FSImage.html.style.overflow;
	FSImage.overflowBody = FSImage.body.style.overflow;
	FSImage.html.style.overflow = 'hidden';
	FSImage.body.style.overflow = 'hidden';
	FSImage.imgID = id;
	FSImage.UpdateImage();
	FSImage.frame.style.display = 'block';
	FSImage.onkeyupBackup = document.onkeyup;
	document.onkeyup = function(e) { FSImage.KeyUp(e); }
}

FSImage.Hide = function()
{
	FSImage.html.style.overflow = FSImage.overflowHTML;
	FSImage.body.style.overflow = FSImage.overflowBody;
	FSImage.frame.style.display='none';
	document.onkeyup = FSImage.onkeyupBackup;
}

FSImage.UpdateImage = function()
{
	this.img.style.backgroundImage = 'url('+this.imgs[this.imgID].src+')';
}

FSImage.KeyUp = function(event)
{
	if ( ! event ) return true;
	if ( event.keyCode ) {
		switch ( event.keyCode ) {
			case 27: this.Hide(); break; // ESC
			case 37: // left
			case 34:	// page down
				this.imgID--;
				if ( this.imgID < 0 ) this.imgID = this.imgs.length-1;
				this.UpdateImage();
				break;
			case 39: // right
			case 33:	// page up
				this.imgID++;
				if ( this.imgID >= this.imgs.length ) this.imgID=0;
				this.UpdateImage();
				break;
		}
	}
	return true;
}

if (document.readyState === "complete") {
	FSImage.Init();
} else {
	if (document.addEventListener) {
		window.addEventListener("load", FSImage.Init, false);
	} else {
		if (document.attachEvent) {
			window.attachEvent("onload", FSImage.Init);
		}
	}
}