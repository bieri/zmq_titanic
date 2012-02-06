#pragma once
class titanic_audit : public titanic_component
{
public:
  
   titanic_audit( zctx_t* ctx,
                  string  brokername,
                  char*   port,
                  int     hbeat,
                  int     reconn );

  ~titanic_audit(void);
  
  void Start();

private:
  
  void* pub_socket;
};
