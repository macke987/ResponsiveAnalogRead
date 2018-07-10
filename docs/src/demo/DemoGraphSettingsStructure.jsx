// @flow
import type {ComponentType} from 'react';
import type {Node} from 'react';
import type Parcel from 'parcels-react';

import React from 'react';
import {PureParcel} from 'parcels-react';
import {Grid, GridItem, Input, Select, Text} from 'dcme-style';
import Structure from '../component/Structure';
import {changeToNumber} from '../util/ParcelModifiers';

type Props = {
    demoParcel: Parcel,
    layout?: ComponentType
};

type LayoutProps = {
    max: () => Node,
    min: () => Node,
    zoom: () => Node
};

const ZOOM_OPTIONS = [
    {value: "1", label: "1x"},
    {value: "2", label: "2x"},
    {value: "4", label: "4x"},
    {value: "8", label: "8x"},
    {value: "16", label: "16x"}
];

export default class DemoGraphSettingsStructure extends Structure<Props> {

    static elements = ['max', 'min', 'zoom'];

    static layout = ({max, min, zoom}: LayoutProps): Node => {
        return <Grid modifier="auto">
            <GridItem />
            <GridItem modifier="paddingMilli shrink">{zoom()}</GridItem>
            <GridItem modifier="paddingMilli shrink">{min()}</GridItem>
            <GridItem modifier="paddingMilli shrink">{max()}</GridItem>
        </Grid>;
    };

    max = (): Node => {
        let {demoParcel} = this.props;
        return <label>
            <Text modifier="monospace marginRight">max</Text>
            <PureParcel parcel={demoParcel.get('max').modify(changeToNumber)} debounce={250}>
                {(parcel) => <Input
                    {...parcel.spread()}
                    type="number"
                    style={{width: '6rem'}}
                />}
            </PureParcel>
        </label>;
    };

    min = (): Node => {
        let {demoParcel} = this.props;
        return <label>
            <Text modifier="monospace marginRight">min</Text>
            <PureParcel parcel={demoParcel.get('min').modify(changeToNumber)} debounce={250}>
                {(parcel) => <Input
                    {...parcel.spread()}
                    type="number"
                    style={{width: '6rem'}}
                />}
            </PureParcel>
        </label>;
    };

    zoom = (): Node => {
        let {demoParcel} = this.props;
        return <label>
            <Text modifier="monospace marginRight">zoom</Text>
            <PureParcel parcel={demoParcel.get('zoom').modify(changeToNumber)}>
                {(parcel) => <Select
                    {...parcel.spread()}
                    options={ZOOM_OPTIONS}
                    clearable={false}
                />}
            </PureParcel>
        </label>;
    };
}
