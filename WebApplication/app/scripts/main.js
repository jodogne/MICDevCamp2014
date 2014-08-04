


	$(document).ready(function(){

		/*$('body').on('click','[rel=lightbox]',function(e){
			e.preventDefault();
			console.log(this);
			$.fancybox([{ href: $(this).attr('href') }]);

		});*/

		$('nav.main .navbar-collapse').on('click','a',function(){
			$('nav.main .navbar-collapse').removeClass('in');
		});

	});

