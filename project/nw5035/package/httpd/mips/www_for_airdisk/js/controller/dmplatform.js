function onBack(){
	var ret = 2;
	if(hasDialog())
	{
		if(isDelaying())
		{
			ret = 2;
		}
		else
		{
			//web处理掉消息
			dismissDialog();
			ret = 1;
		}
	}
	else
	{
		//返回给app自行处理
		ret = 2;
	}
	return ret;
}


