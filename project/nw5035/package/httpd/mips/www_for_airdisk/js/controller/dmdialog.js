var dialogFlag=0;
function hasDialog()
{
	if(dialogFlag != 0)
		return true;
	else
		return false;
}
function dismissDialog()
{
	layer.closeAll();
	dialogFlag = 0;
}

function initLayer()
{
	layer.config({
	    success: function(layero, index){
			console.log(layero, index);
			dialogFlag++;
		},
		end: function(layero, index){
			console.log(layero, index);
			dialogFlag--;
		}
	});
}