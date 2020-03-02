int main(void)
{
    char* pw = "password";
	char buf[16];
	char* ptr = buf;
	puts("Whatisthepassword?");
	gets(buf, 15);
	while (*pw && *buf)
		if (*pw++ != *buf++)
			return 1;
	if (*pw != *buf)
		return 1;
	INT(0x7F);
	return 0;
}