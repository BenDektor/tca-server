

class Client {
public:
    Client(const char *ip, int port1, int port2);
    void connectToServer();

private:
    const char *ip;
    int port1, port2;

    void connectToPort(int port);
};

