import { useState, useEffect, useRef } from 'react'
import Drawer from '@mui/material/Drawer';
import Box from '@mui/material/Box';
import AppBar from './AppBar'
import Files from './Files'
import _ from 'lodash'
import { useAtom } from 'jotai'
import { fileAtom, uploadAtom, uploadCountAtom } from './atom'
import './App.css'
import util from "./util";
function ws_uri() {
  let loc = window.location, ws_uri, h = loc.host;
  if (loc.protocol === "https:") {
    ws_uri = "wss:";
  } else if (loc.protocol === "http:") {
    ws_uri = "ws:";
  } else {
    ws_uri = "ws:";
    h = `localhost:8888`;
  }
  if( import.meta.env.DEV ) {
    h = `192.168.0.60:8888`;
    // console.log(`[ws_uri] app is running in development mode`)
  } else {
    // console.log(`[ws_uri] app is running in production mode`)
  }
    
  ws_uri += "//" + h + "/store";
  console.log(ws_uri)
  return ws_uri;
}
function progress_cap(f){
  return `${util.truncate(f.name)}${util.formatFileSize(f.size)}`
}
function App() {
  const [ , setFile ] = useAtom(fileAtom)
  const [ upload, setUpload ] = useAtom(uploadAtom)
  const [ count, setCount ] = useAtom(uploadCountAtom)
  // const [count, setCount] = useState(_.size(upload));
  const uploadRef = useRef();
  uploadRef.current = upload;
  useEffect(() => {
    const ws = new WebSocket(ws_uri());
    ws.onmessage = function (event) {
      try{
        const data = JSON.parse(event.data)
        // console.log(data)
        setFile(data)
      }catch(err){
        console.log(event.data)
      }
    };
    //clean up function
    return () => ws.close();
  }, []);
  // do not use brace
  const progressBars = _.map(uploadRef.current, f=>
    <div className="progressbar" key={f.name}>
      <div style={{width: f.progress}}></div>
      <div className="cap">{`${progress_cap(f)}(${f.progress})`}</div>
    </div>
  )
  // console.log(`progressBars=`,progressBars)
  return (
    <>
      <AppBar />
      <Files />
      <Drawer
            anchor='bottom'
            // sx={{overflow: 'auto'}}
            open={count > 0}
            onClose={()=>{
              console.log(`count=${count}`)
            }}
          >
            <Box sx={{ overflow: 'auto', width: '100%', 
              backgroundColor: 'rgba(0, 0, 0, .25)', maxHeight: '80vh' }}>
            {progressBars}
            </Box>
          </Drawer>
    </>
  )
}

export default App
