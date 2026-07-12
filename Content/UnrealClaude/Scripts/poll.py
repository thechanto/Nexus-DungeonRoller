import json,urllib.request,sys,time

BASE="http://localhost:3000/mcp/tool/"

def call(route, payload, timeout=12):
    req=urllib.request.Request(BASE+route,data=json.dumps(payload).encode(),headers={"Content-Type":"application/json"})
    return urllib.request.urlopen(req,timeout=timeout).read().decode()

tid=sys.argv[1]
deadline=time.time()+540
last=""
while time.time()<deadline:
    try:
        r=call("task_status",{"task_id":tid})
        d=json.loads(r)
        st=json.dumps(d.get("data",d)).lower()
        if '"completed"' in st or '"failed"' in st or '"error"' in st:
            print("STATUS_TERMINAL", r)
            try:
                print("RESULT", call("task_result",{"task_id":tid},timeout=20))
            except Exception as e:
                print("RESULT_ERR", e)
            sys.exit(0)
        last=r
    except Exception as e:
        last="poll_timeout:%s"%e
    time.sleep(4)
print("POLL_DEADLINE", last)
