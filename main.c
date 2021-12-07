#include <stdio.h>
#include <X11/Xlib.h>

static int cbret = 0;

int _enum_windows(Display *display, Window root_window, int (*callback)(Window window, void *arg), void *arg)
{
	Window tmp_window;
	Window *window_list;
	unsigned int window_count;
	unsigned int i;

	if (cbret)
		return 0;

	XQueryTree(
		display, root_window, &tmp_window, &tmp_window, &window_list,
		&window_count
	);

	if (window_count == 0)
		return 0;

	for (i = 0; i < window_count; ++i) {
		cbret = callback(window_list[i], arg);
		if (cbret)
			break;
		
		if (_enum_windows(display, window_list[i], callback, arg))
			break;
	}

	XFree(window_list);
	return cbret;
}

int enum_windows(Display *display, Window root_window, int (*callback)(Window window, void *arg), void *arg)
{
	int ret;

	ret = _enum_windows(display, root_window, callback, arg);
	cbret = 0;
	return 0;
}

int window_callback(Window window, void *arg)
{
	Display *display = (Display *)arg;
	pid_t pid;
	char *window_name;
	Atom property;
	Atom atom_tmp;
	int i_tmp;
	unsigned long ul_tmp;
	unsigned char *pid_data;

	property = XInternAtom(display, "_NET_WM_PID", True);

	printf("[*] Window ID: %d\n", window);
	if (XFetchName(display, window, &window_name) >= Success) {
		printf("[*] Window Name: %s\n", window_name);
		XFree(window_name);
	}
	
	if (XGetWindowProperty(display, window, property, 0, BUFSIZ,
		False, AnyPropertyType, &atom_tmp, &i_tmp, &ul_tmp, &ul_tmp,
		&pid_data
	)) {
		pid = *(pid_t *)pid_data;
		printf("[*] Process ID: %d\n", pid);
		XFree(pid_data);
	}
	printf("====================\n");

	return 0;
}

int main()
{
	Display *display;
	Window root_window;

	display = XOpenDisplay(NULL);
	root_window = RootWindow(display, DefaultScreen(display));
	enum_windows(display, root_window, window_callback, (void *)display);

	return 0;
}