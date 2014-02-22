/*
 *  shipping labels
 */
before_request()
{
	return;
}
before_each_page()
{
	set_size(5);
	nplace(qi.paper_order, "99999", 1, 1);
	set_size(1);
}
