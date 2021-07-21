(function($) {
  "use strict"; // Start of use strict

  // Smooth scrolling using jQuery easing
  $('a.js-scroll-trigger[href*="#"]:not([href="#"])').click(function() {
    if (location.pathname.replace(/^\//, '') == this.pathname.replace(/^\//, '') && location.hostname == this.hostname) {
      var target = $(this.hash);
      target = target.length ? target : $('[name=' + this.hash.slice(1) + ']');
      if (target.length) {
        $('html, body').animate({
          scrollTop: (target.offset().top - 72)
        }, 1000, "easeInOutExpo");
        return false;
      }
    }
  });

  // Closes responsive menu when a scroll trigger link is clicked
  $('.js-scroll-trigger').click(function() {
    $('.navbar-collapse').collapse('hide');
  });

  // Activate scrollspy to add active class to navbar items on scroll
  $('body').scrollspy({
    target: '#mainNav',
    offset: 75
  });

  // Collapse Navbar
  var navbarCollapse = function() {
    if ($("#mainNav").offset().top > 100) {
      $("#mainNav").addClass("navbar-scrolled");
    } else {
      $("#mainNav").removeClass("navbar-scrolled");
    }
  };
  
    navbarCollapse();

  $(window).scroll(navbarCollapse);


})(jQuery);


//Color picker

let colorPicker = new iro.ColorPicker("#colorPicker", {
  width: 300,
  color: "rgb(255, 0, 0)",
  borderWidth: 3,
  borderColor: "#fff",
});



//WebSocket
let timerWebSocket = 0;
let webSocket;
let lastSend = 0;
initWebSocket();  

function initWebSocket() {
    webSocket = new WebSocket("ws:" + "/" + window.location.hostname + ':81/');
    webSocket.onopen = function (evt) {
      console.log('WebSocket open');

        if (timerWebSocket) {
            clearInterval(timerWebSocket);
            timerWebSocket = 0;
        }
    };
    webSocket.onclose = function (evt) {
        console.log('WebSocket close');

        webSocket = null;
        if (!timerWebSocket)
            timerWebSocket = setInterval(function () {console.log("WebSocket retry"); initWebSocket(); }, 1000);
    };
    webSocket.onerror = function (evt) {console.log(evt); location.reload()};
    webSocket.onmessage = function (evt) {
        console.log(evt);

		
        if (evt.data[0] === 'I') {  //Initialization
            let words = evt.data.substring(1).split(",");
            let current = 0;

			colorPicker.color.set({r:parseInt(words[current++]),g:parseInt(words[current++]),b:parseInt(words[current++])});

            document.getElementById("btn-rest-a").style.borderColor = rgb(parseInt(words[current++]), parseInt(words[current++]), parseInt(words[current++]));
            
            if (words[current++] == 1) {
              $("#btn-change-a").removeClass("btn-secondary").addClass("btn-success");
            }
			
			if (words[current++] == 1) {
              $("#btn-change-b").removeClass("btn-secondary").addClass("btn-warning");
			  $("#btn-change-b").text("Turn OFF LED's");
            }
			
			if (words[current++] == 1) {
              $("#btn-change-c").removeClass("btn-secondary").addClass("btn-success");
            }
            
        }
		
        else if (evt.data[0] === 'A'){ 
            let words = evt.data.substring(1).split(",");
            let current = 0;
			colorPicker.color.set({r:parseInt(words[current++]),g:parseInt(words[current++]),b:parseInt(words[current++])});
        }
		
		else if (evt.data[0] === 'B'){ 
			let words = evt.data.substring(1).split(",");
            let current = 0;
			document.getElementById("btn-rest-a").style.borderColor = rgb(parseInt(words[current++]), parseInt(words[current++]), parseInt(words[current++]));
		}
		
        else if (evt.data[0] === 'C')
          if(evt.data.substring(1) == 1)
            $("#btn-change-a").removeClass("btn-secondary").addClass("btn-success");
          else
            $("#btn-change-a").removeClass("btn-success").addClass("btn-secondary");
		
		else if (evt.data[0] === 'D')
          if(evt.data.substring(1) == 1){
              $("#btn-change-b").removeClass("btn-secondary").addClass("btn-warning");
			  $("#btn-change-b").text("Turn OFF LED's");
		  }
          else{
            $("#btn-change-b").removeClass("btn-warning").addClass("btn-secondary");
			$("#btn-change-b").text("Turn ON LED's");
		  }
		
		else if (evt.data[0] === 'E')
          if(evt.data.substring(1) == 1)
            $("#btn-change-c").removeClass("btn-secondary").addClass("btn-success");
          else
            $("#btn-change-c").removeClass("btn-success").addClass("btn-secondary");

        else 
          console.log('Unknown event');
    };
}

function sendButton(button) {
    webSocket?.send('b' + button);
}

function rgb(r, g, b){
  return "rgb("+r+","+g+","+b+")";
}

colorPicker.on(["color:change"], function(color){
    let now = (new Date).getTime();
    if (lastSend > now - 20) return;
    lastSend = now;
    webSocket?.send('a' + color.hexString.substring(1));
});

function resetFrontColor(){
	colorPicker.color.set({r:255,g:0,b:0});
	webSocket?.send("aff0000");
}
