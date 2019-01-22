using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class BoxImageLoader : MonoBehaviour {

    public int index;
    public bool isEquip = false;
    private RawImage img;

	// Use this for initialization
	void Start () {
        img = GetComponent<RawImage>();
    }
	
	// Update is called once per frame
	void Update () {
        if (isEquip)
        {
            if(PlayableDatabase.Getinst.GetListCount(0) > index)
            {
                img.texture = Resources.Load<Texture>("Image/ItemIcon/C_" + PlayableDatabase.Getinst.GetChara(index).ID);
            }

            else
            {
                img.texture = Resources.Load<Texture>("Image/ItemIcon/C_-1");
            }
        }

        else
        {
            if (PlayableDatabase.Getinst.GetListCount(1) > index)
            {
                img.texture = Resources.Load<Texture>("Image/ItemIcon/C_" + PlayableDatabase.Getinst.GetBoxChara(index).ID);
            }

            else
            {
                img.texture = Resources.Load<Texture>("Image/ItemIcon/C_-1");
            }
        }
    }
}
