import { atom } from 'jotai'

const fileAtom = atom([])
const uploadAtom = atom({})
const filterAtom = atom('')
const uploadCountAtom = atom(0)
export {
    fileAtom,
    uploadAtom,
    uploadCountAtom,
    filterAtom,
}