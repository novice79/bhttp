import React from 'react'
import { useAtom } from 'jotai'
import { fileAtom, filterAtom } from './atom'
import Box from '@mui/material/Box';
import { Download, Delete } from '@mui/icons-material';
import util from "./util";


export default function Files() {
    const [ files, setFile ] = useAtom(fileAtom)
    const [ filterTxt, setFilterTxt ] = useAtom(filterAtom)
    const listItems = files
        .filter(fi=>filterTxt? fi.name.includes(filterTxt) && fi : fi)
        .map( fi =>
        <React.Fragment key={fi.name}>
            <Box sx={{ 
                width: '100%', 
                display: 'flex', flexWrap: 'wrap', alignItems: 'center',
                backgroundColor: 'rgb(177, 250, 250)',
                border: '.3rem groove',
                marginTop: '.4rem' }}>
                <Box sx={{ width: '60%', overflowWrap: 'break-word' }}>{fi.name}</Box>
                <Box sx={{ width: '20%', overflowWrap: 'break-word' }}>{util.formatFileSize(fi.size)}</Box>
                <Box sx={{ 
                    width: '20%',
                    display: 'flex', 
                    justifyContent: 'flex-end'
                    }}>
                    <a href={import.meta.env.DEV? 
                        `http://192.168.0.60:8888/store/${fi.name}` 
                        :`/store/${fi.name}`} download={fi.name}
                        target="_blank" rel="noopener noreferrer"><Download sx={{ mr: 1 }}/></a>
                    <Delete sx={{ mr: 1 }} onClick={()=>{
                        const msg = `Are you sure to delete:\n${fi.name}`
                        if (confirm(msg) == true) {
                            const url = import.meta.env.DEV? 
                            `http://192.168.0.60:8888/del` 
                            :`/del`
                            util.post_data(url, fi.path);
                        } else {

                        }
                    }}/>
                </Box>
            </Box>
        </React.Fragment>
    );
    return (
        <Box sx={{ width: '100%', marginTop: '4.1rem'}}>
            {listItems}
        </Box>
    );
}
